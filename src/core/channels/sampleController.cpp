/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
 *
 * This file is part of Giada - Your Hardcore Loopmachine.
 *
 * Giada - Your Hardcore Loopmachine is free software: you can
 * redistribute it and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation, either
 * version 3 of the License, or (at your option) any later version.
 *
 * Giada - Your Hardcore Loopmachine is distributed in the hope that it
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------- */


#include <cassert>
#include "core/conf.h"
#include "core/clock.h"
#include "core/action.h"
#include "core/mixer.h"
#include "core/channels/state.h"
#include "utils/math.h"
#include "sampleController.h"


namespace giada {
namespace m 
{
namespace
{
constexpr int Q_ACTION_PLAY   = 0;
constexpr int Q_ACTION_REWIND = 1;
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


SampleController::SampleController(ChannelState* c, SamplePlayerState* s)
: m_channelState     (c)
, m_samplePlayerState(s) 
{
	m_samplePlayerState->quantizer.schedule(Q_ACTION_PLAY, 
		[&status = m_channelState->playStatus, &offset = m_samplePlayerState->offset]
		(Frame delta)
	{
		offset = delta;
		status = ChannelStatus::PLAY;
	});

	m_samplePlayerState->quantizer.schedule(Q_ACTION_REWIND, [this] (Frame delta)
	{
		rewind(delta);
	});
}


/* -------------------------------------------------------------------------- */


SampleController::SampleController(const SampleController& o, ChannelState* c, SamplePlayerState* s)
: SampleController(c, s)
{
}


/* -------------------------------------------------------------------------- */


void SampleController::parse(const mixer::Event& e) const
{
	assert(m_channelState      != nullptr);
	assert(m_samplePlayerState != nullptr);

	switch (e.type) {
		case mixer::EventType::KEY_PRESS:
			press(e.delta, e.action.event.getVelocity(), /*manual=*/true); break;

		case mixer::EventType::KEY_RELEASE:
			release(e.delta); break;

		case mixer::EventType::KEY_KILL:
			kill(e.delta); break;

		case mixer::EventType::SEQUENCER_FIRST_BEAT:
			if (clock::isRunning())
				onFirstBeat(e.delta); 
			break;
		
		case mixer::EventType::SEQUENCER_BAR:
			onBar(e.delta);  break;
		
		case mixer::EventType::SEQUENCER_STOP:	
			onStopBySeq(); break;
		
		case mixer::EventType::ACTION:
			if (m_channelState->readActions.load() == true)
				parseAction(e.action, e.delta);
			break;
			
		case mixer::EventType::CHANNEL_TOGGLE_READ_ACTIONS:	
			toggleReadActions(); break;   
		
		default: break;
	}
}


/* -------------------------------------------------------------------------- */


void SampleController::onLastFrame() const
{
    ChannelStatus    playStatus = m_channelState->playStatus.load();
    SamplePlayerMode mode       = m_samplePlayerState->mode.load();
    bool             running    = clock::isRunning();
	
    if (playStatus == ChannelStatus::PLAY) {
        /* Stop LOOP_* when the sequencer is off, or SINGLE_* except for
        SINGLE_ENDLESS, which runs forever unless it's in ENDING mode. 
        Other loop once modes are put in wait mode. */
        if ((mode == SamplePlayerMode::SINGLE_BASIC   || 
             mode == SamplePlayerMode::SINGLE_PRESS   ||
             mode == SamplePlayerMode::SINGLE_RETRIG) || 
            (m_samplePlayerState->isAnyLoopMode() && !running))
            playStatus = ChannelStatus::OFF;
        else
        if (mode == SamplePlayerMode::LOOP_ONCE || mode == SamplePlayerMode::LOOP_ONCE_BAR)
            playStatus = ChannelStatus::WAIT;
    }
    else
    if (playStatus == ChannelStatus::ENDING) {
		/* LOOP_ONCE or LOOP_ONCE_BAR: if ending (i.e. the user requested 
		their termination), stop 'em. Let them wait otherwise. */
		if (mode == SamplePlayerMode::LOOP_ONCE || mode == SamplePlayerMode::LOOP_ONCE_BAR)
			playStatus = ChannelStatus::WAIT;			
        else
            playStatus = ChannelStatus::OFF;
	}

    m_channelState->playStatus.store(playStatus);	
}


/* -------------------------------------------------------------------------- */


void SampleController::advance(Frame bufferSize) const
{
	Range<Frame> block(clock::getCurrentFrame(), clock::getCurrentFrame() + bufferSize);
	m_samplePlayerState->quantizer.advance(block, clock::getQuantizerStep());
}


/* -------------------------------------------------------------------------- */


void SampleController::press(Frame localFrame, int velocity, bool manual) const
{
    ChannelStatus    playStatus = m_channelState->playStatus.load();
    SamplePlayerMode mode       = m_samplePlayerState->mode.load();
	bool             isLoop     = m_samplePlayerState->isAnyLoopMode();

    switch (playStatus) {
		case ChannelStatus::OFF:
			playStatus = pressWhileOff(localFrame, velocity, isLoop, manual); break;

		case ChannelStatus::PLAY:
			playStatus = pressWhilePlay(localFrame, mode, isLoop, manual); break;

		case ChannelStatus::WAIT:
			playStatus = ChannelStatus::OFF; break;

		case ChannelStatus::ENDING:
			playStatus = ChannelStatus::PLAY; break;

		default: break;
	}

    m_channelState->playStatus.store(playStatus); 
}


/* -------------------------------------------------------------------------- */


void SampleController::release(Frame localFrame) const
{
	/* Key release is meaningful only for SINGLE_PRESS modes. */
	
	if (m_samplePlayerState->mode.load() != SamplePlayerMode::SINGLE_PRESS)
		return;

	/* Kill it if it's SINGLE_PRESS is playing. Otherwise there might be a 
	quantization step in progress that would play the channel later on: 
	disable it. */

	if (m_channelState->playStatus.load() == ChannelStatus::PLAY)
		kill(localFrame);
	else
    if (m_samplePlayerState->quantizer.isTriggered())
        m_samplePlayerState->quantizer.clear();
}


/* -------------------------------------------------------------------------- */


void SampleController::kill(Frame localFrame) const
{
    m_channelState->playStatus.store(ChannelStatus::OFF);
    m_samplePlayerState->tracker.store(m_samplePlayerState->begin.load());
    m_samplePlayerState->quantizing = false; 

    /*  Clear data in range [localFrame, (buffer.size)) if the kill event occurs
    in the middle of the buffer. */

    if (localFrame != 0)
        m_channelState->buffer.clear(localFrame);
}


/* -------------------------------------------------------------------------- */


void SampleController::rewind(Frame localFrame) const
{
	/* Quantization stops on rewind. */

	m_samplePlayerState->quantizer.clear(); 

	if (m_channelState->isPlaying()) { 
		m_samplePlayerState->rewinding = true;
		m_samplePlayerState->offset    = localFrame;
	}
	else
		m_samplePlayerState->tracker.store(m_samplePlayerState->begin.load());
}


/* -------------------------------------------------------------------------- */


ChannelStatus SampleController::pressWhileOff(Frame localFrame, int velocity, bool isLoop, bool manual) const
{
	m_samplePlayerState->offset = localFrame;

	if (isLoop)
		return ChannelStatus::WAIT;

	if (m_samplePlayerState->velocityAsVol.load() == true)	
		m_channelState->volume_i = u::math::map(velocity, G_MAX_VELOCITY, G_MAX_VOLUME); 

	if (clock::canQuantize() && manual) { // manual: don't quantize recorded actions
		m_samplePlayerState->quantizer.trigger(Q_ACTION_PLAY);
		return ChannelStatus::OFF;
	}
	else
		return ChannelStatus::PLAY;
}


ChannelStatus SampleController::pressWhilePlay(Frame localFrame, SamplePlayerMode mode, bool isLoop, bool manual) const
{
	if (mode == SamplePlayerMode::SINGLE_RETRIG) {
		if (clock::canQuantize() && manual)  // manual: don't quantize recorded actions 
			m_samplePlayerState->quantizer.trigger(Q_ACTION_REWIND);
		else
			rewind(localFrame);
		return ChannelStatus::PLAY;
	}

	if (isLoop || mode == SamplePlayerMode::SINGLE_ENDLESS)
		return ChannelStatus::ENDING;

	if (mode == SamplePlayerMode::SINGLE_BASIC) {
		rewind(localFrame);
		return ChannelStatus::OFF;
	}

	return ChannelStatus::OFF;
}


/* -------------------------------------------------------------------------- */


void SampleController::onBar(Frame localFrame) const
{
	G_DEBUG("onBar ch=" << m_channelState->id);

    ChannelStatus    playStatus = m_channelState->playStatus.load();
    SamplePlayerMode mode       = m_samplePlayerState->mode.load();

    if (playStatus == ChannelStatus::PLAY && mode == SamplePlayerMode::LOOP_REPEAT)
        rewind(localFrame);
    else
    if (playStatus == ChannelStatus::WAIT && mode == SamplePlayerMode::LOOP_ONCE_BAR) {
		m_channelState->playStatus.store(ChannelStatus::PLAY);
        m_samplePlayerState->offset = localFrame;
	}
}


/* -------------------------------------------------------------------------- */


void SampleController::onFirstBeat(Frame localFrame) const
{
G_DEBUG("onFirstBeat ch=" << m_channelState->id << ", localFrame=" << localFrame);

    ChannelStatus playStatus = m_channelState->playStatus.load();
	bool          isLoop     = m_samplePlayerState->isAnyLoopMode();

	switch (playStatus) { 

		case ChannelStatus::PLAY:
			if (isLoop) 
				rewind(localFrame); 
			break;
		
		case ChannelStatus::WAIT:
        	m_samplePlayerState->offset = localFrame;
            m_channelState->playStatus.store(ChannelStatus::PLAY);
			break;

		case ChannelStatus::ENDING:
			if (isLoop) 
				kill(localFrame);
			break;
		
		default: break;
	}
}


/* -------------------------------------------------------------------------- */


void SampleController::onStopBySeq() const
{
	G_DEBUG("onStopBySeq ch=" << m_channelState->id);

	ChannelStatus playStatus       = m_channelState->playStatus.load();
	bool          isReadingActions = m_channelState->readActions.load() == true;
	bool          isLoop           = m_samplePlayerState->isAnyLoopMode();

	switch (playStatus) {

		case ChannelStatus::WAIT:
			/* Loop-mode channels in wait status get stopped right away. */
			if (isLoop)
				m_channelState->playStatus.store(ChannelStatus::OFF);	
			break;

		case ChannelStatus::PLAY:
			/* Kill samples if a) chansStopOnSeqHalt == true (run the sample to end 
			otherwise); b) when a channel is reading (and playing) actions. */
			if (conf::conf.chansStopOnSeqHalt)
				if (isLoop || isReadingActions)
					kill(0);
			break;

		default: break;
	}
}


/* -------------------------------------------------------------------------- */


void SampleController::parseAction(const Action& a, Frame localFrame) const
{
	bool isLoop = m_samplePlayerState->isAnyLoopMode();

	switch (a.event.getStatus()) {
		case MidiEvent::NOTE_ON:
			if (!isLoop)
				press(localFrame, /*velocity=*/G_MAX_VELOCITY, /*manual=*/false);
			break;
		case MidiEvent::NOTE_OFF:
			if (!isLoop)
				release(localFrame);
			break;
		case MidiEvent::NOTE_KILL:
			if (!isLoop)
				kill(localFrame);
			break;
		case MidiEvent::ENVELOPE:
			//calcVolumeEnv_(ch, a); TODO
			break;
	}
}


/* -------------------------------------------------------------------------- */


void SampleController::toggleReadActions() const
{
	ChannelStatus recStatus = m_channelState->recStatus.load();
	if (clock::isRunning() && recStatus == ChannelStatus::PLAY && !conf::conf.treatRecsAsLoops)
		kill(0);
}
}} // giada::m::
