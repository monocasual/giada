/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual

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
#include "src/core/model/model.h"
#include "core/channels/channel.h"
#include "core/conf.h"
#include "core/clock.h"
#include "utils/math.h"
#include "sampleReactor.h"


namespace giada::m::sampleReactor
{
namespace
{
constexpr int Q_ACTION_PLAY   = 0;
constexpr int Q_ACTION_REWIND = 1;

void          press_            (channel::Data& ch,int velocity);
void          release_          (channel::Data& ch);
void          kill_             (channel::Data& ch);
void          onStopBySeq_      (channel::Data& ch);
void          toggleReadActions_(channel::Data& ch);
ChannelStatus pressWhileOff_    (channel::Data& ch, int velocity, bool isLoop);
ChannelStatus pressWhilePlay_   (channel::Data& ch, SamplePlayerMode mode, bool isLoop);
void          rewind_           (channel::Data& ch, Frame localFrame=0);


/* -------------------------------------------------------------------------- */


void press_(channel::Data& ch, int velocity)
{
    ChannelStatus    playStatus = ch.state->playStatus.load();
    SamplePlayerMode mode       = ch.samplePlayer->mode;
	bool             isLoop     = ch.samplePlayer->isAnyLoopMode();

    switch (playStatus) {
		case ChannelStatus::OFF:
			playStatus = pressWhileOff_(ch, velocity, isLoop); break;

		case ChannelStatus::PLAY:
			playStatus = pressWhilePlay_(ch, mode, isLoop); break;

		case ChannelStatus::WAIT:
			playStatus = ChannelStatus::OFF; break;

		case ChannelStatus::ENDING:
			playStatus = ChannelStatus::PLAY; break;

		default: break;
	}

	ch.state->playStatus.store(playStatus);
}


/* -------------------------------------------------------------------------- */


void release_(channel::Data& ch)
{
	/* Key release is meaningful only for SINGLE_PRESS modes. */
	
	if (ch.samplePlayer->mode != SamplePlayerMode::SINGLE_PRESS)
		return;

	/* Kill it if it's SINGLE_PRESS is playing. Otherwise there might be a 
	quantization step in progress that would play the channel later on: 
	disable it. */

	if (ch.state->playStatus.load() == ChannelStatus::PLAY)
		kill_(ch);
	else
    if (sequencer::quantizer.hasBeenTriggered())
    	sequencer::quantizer.clear();
}


/* -------------------------------------------------------------------------- */


void kill_(channel::Data& ch)
{
    ch.state->playStatus.store(ChannelStatus::OFF);
    ch.state->tracker.store(ch.samplePlayer->begin);
}


/* -------------------------------------------------------------------------- */


void onStopBySeq_(channel::Data& ch)
{
	G_DEBUG("onStopBySeq ch=" << ch.id);

	ChannelStatus playStatus       = ch.state->playStatus.load();
	bool          isReadingActions = ch.readActions;
	bool          isLoop           = ch.samplePlayer->isAnyLoopMode();

	switch (playStatus) {

		case ChannelStatus::WAIT:
			/* Loop-mode channels in wait status get stopped right away. */
			if (isLoop)
                ch.state->playStatus.store(ChannelStatus::OFF);
			break;

		case ChannelStatus::PLAY:
			if (conf::conf.chansStopOnSeqHalt && (isLoop || isReadingActions))
				kill_(ch);
			break;

		default: break;
	}
}


/* -------------------------------------------------------------------------- */


ChannelStatus pressWhileOff_(channel::Data& ch, int velocity, bool isLoop)
{
	if (isLoop)
		return ChannelStatus::WAIT;

	if (ch.samplePlayer->velocityAsVol)
		ch.volume_i = u::math::map(velocity, G_MAX_VELOCITY, G_MAX_VOLUME);

	if (clock::canQuantize()) {
		sequencer::quantizer.trigger(Q_ACTION_PLAY + ch.id);
		return ChannelStatus::OFF;
	}
	else
		return ChannelStatus::PLAY;
}


/* -------------------------------------------------------------------------- */


ChannelStatus pressWhilePlay_(channel::Data& ch, SamplePlayerMode mode, bool isLoop)
{
	if (mode == SamplePlayerMode::SINGLE_RETRIG) {
		if (clock::canQuantize())
			sequencer::quantizer.trigger(Q_ACTION_REWIND + ch.id);
		else
			rewind_(ch);
		return ChannelStatus::PLAY;
	}

	if (isLoop || mode == SamplePlayerMode::SINGLE_ENDLESS)
		return ChannelStatus::ENDING;

	if (mode == SamplePlayerMode::SINGLE_BASIC) {
		rewind_(ch);
		return ChannelStatus::OFF;
	}

	return ChannelStatus::OFF;
}


/* -------------------------------------------------------------------------- */


void toggleReadActions_(channel::Data& ch)
{
	if (clock::isRunning() && ch.state->recStatus.load() == ChannelStatus::PLAY && !conf::conf.treatRecsAsLoops)
		kill_(ch);
}


/* -------------------------------------------------------------------------- */


void rewind_(channel::Data& ch, Frame localFrame)
{
	if (ch.isPlaying()) { 
		ch.state->rewinding = true;
		ch.state->offset    = localFrame;
	}
	else
		ch.state->tracker.store(ch.samplePlayer->begin);
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


Data::Data(ID channelId)
{
	sequencer::quantizer.schedule(Q_ACTION_PLAY + channelId, [channelId] (Frame delta)
	{
        channel::Data& ch = model::get().getChannel(channelId);
		ch.state->offset = delta;
		ch.state->playStatus.store(ChannelStatus::PLAY);
	});

	sequencer::quantizer.schedule(Q_ACTION_REWIND + channelId, [channelId] (Frame delta)
	{
        channel::Data& ch = model::get().getChannel(channelId);
		rewind_(ch, delta);
	});
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void react(channel::Data& ch, const eventDispatcher::Event& e)
{
	if (!ch.hasWave())
		return;

	switch (e.type) {

		case eventDispatcher::EventType::KEY_PRESS:
			press_(ch, std::get<int>(e.data)); break;

		case eventDispatcher::EventType::KEY_RELEASE:
			release_(ch); break;

		case eventDispatcher::EventType::KEY_KILL:
			kill_(ch); break;

		case eventDispatcher::EventType::SEQUENCER_STOP:	
			onStopBySeq_(ch); break;

		case eventDispatcher::EventType::CHANNEL_TOGGLE_READ_ACTIONS:	
			toggleReadActions_(ch); break;

		default: break;
	}
}
} // giada::m::sampleReactor::