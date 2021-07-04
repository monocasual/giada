/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#include "sequencer.h"
#include "core/clock.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/kernelAudio.h"
#include "core/metronome.h"
#include "core/mixer.h"
#include "core/model/model.h"
#include "core/quantizer.h"
#include "core/recManager.h"

namespace giada::m::sequencer
{
namespace
{
constexpr int Q_ACTION_REWIND = 0;

/* eventBuffer_
Buffer of events found in each block sent to channels for event parsing. This is 
filled during react(). */

EventBuffer eventBuffer_;

Metronome metronome_;

/* -------------------------------------------------------------------------- */

void rewindQ_(Frame delta)
{
	clock::rewind();
	eventBuffer_.push_back({EventType::REWIND, 0, delta});
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Quantizer quantizer;

/* -------------------------------------------------------------------------- */

void init()
{
	quantizer.schedule(Q_ACTION_REWIND, rewindQ_);
	clock::rewind();
}

/* -------------------------------------------------------------------------- */

void react(const eventDispatcher::EventBuffer& events)
{
	for (const eventDispatcher::Event& e : events)
	{
		if (e.type == eventDispatcher::EventType::SEQUENCER_START)
		{
			start();
			break;
		}
		if (e.type == eventDispatcher::EventType::SEQUENCER_STOP)
		{
			stop();
			break;
		}
		if (e.type == eventDispatcher::EventType::SEQUENCER_REWIND)
		{
			rewind();
			break;
		}
	}
}

/* -------------------------------------------------------------------------- */

const EventBuffer& advance(Frame bufferSize)
{
	eventBuffer_.clear();

	const Frame start        = clock::getCurrentFrame();
	const Frame end          = start + bufferSize;
	const Frame framesInLoop = clock::getFramesInLoop();
	const Frame framesInBar  = clock::getFramesInBar();
	const Frame framesInBeat = clock::getFramesInBeat();

	for (Frame i = start, local = 0; i < end; i++, local++)
	{

		Frame global = i % framesInLoop; // wraps around 'framesInLoop'

		if (global == 0)
		{
			eventBuffer_.push_back({EventType::FIRST_BEAT, global, local});
			metronome_.trigger(Metronome::Click::BEAT, local);
		}
		else if (global % framesInBar == 0)
		{
			eventBuffer_.push_back({EventType::BAR, global, local});
			metronome_.trigger(Metronome::Click::BAR, local);
		}
		else if (global % framesInBeat == 0)
		{
			metronome_.trigger(Metronome::Click::BEAT, local);
		}

		const std::vector<Action>* as = recorder::getActionsOnFrame(global);
		if (as != nullptr)
			eventBuffer_.push_back({EventType::ACTIONS, global, local, as});
	}

	/* Advance clock and quantizer after the event parsing. */
	clock::advance(bufferSize);
	quantizer.advance(Range<Frame>(start, end), clock::getQuantizerStep());

	return eventBuffer_;
}

/* -------------------------------------------------------------------------- */

void render(mcl::AudioBuffer& outBuf)
{
	if (metronome_.running)
		metronome_.render(outBuf);
}

/* -------------------------------------------------------------------------- */

void rawStart()
{
	switch (clock::getStatus())
	{
	case ClockStatus::STOPPED:
		clock::setStatus(ClockStatus::RUNNING);
		break;
	case ClockStatus::WAITING:
		clock::setStatus(ClockStatus::RUNNING);
		recManager::stopActionRec();
		break;
	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void rawStop()
{
	clock::setStatus(ClockStatus::STOPPED);

	/* If recordings (both input and action) are active deactivate them, but 
	store the takes. RecManager takes care of it. */

	if (recManager::isRecordingAction())
		recManager::stopActionRec();
	else if (recManager::isRecordingInput())
		recManager::stopInputRec(conf::conf.inputRecMode);
}

/* -------------------------------------------------------------------------- */

void rawRewind()
{
	if (clock::canQuantize())
		quantizer.trigger(Q_ACTION_REWIND);
	else
		rewindQ_(/*delta=*/0);
}

/* -------------------------------------------------------------------------- */

void start()
{
#ifdef WITH_AUDIO_JACK
	if (kernelAudio::getAPI() == G_SYS_API_JACK)
		kernelAudio::jackStart();
	else
#endif
		rawStart();
}

/* -------------------------------------------------------------------------- */

void stop()
{
#ifdef WITH_AUDIO_JACK
	if (kernelAudio::getAPI() == G_SYS_API_JACK)
		kernelAudio::jackStop();
	else
#endif
		rawStop();
}

/* -------------------------------------------------------------------------- */

void rewind()
{
#ifdef WITH_AUDIO_JACK
	if (kernelAudio::getAPI() == G_SYS_API_JACK)
		kernelAudio::jackSetPosition(0);
	else
#endif
		rawRewind();
}

/* -------------------------------------------------------------------------- */

bool isMetronomeOn() { return metronome_.running; }
void toggleMetronome() { metronome_.running = !metronome_.running; }
void setMetronome(bool v) { metronome_.running = v; }
} // namespace giada::m::sequencer