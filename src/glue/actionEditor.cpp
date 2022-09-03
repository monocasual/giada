/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "glue/actionEditor.h"
#include "core/actions/action.h"
#include "core/actions/actionRecorder.h"
#include "core/actions/actions.h"
#include "core/const.h"
#include "core/engine.h"
#include "core/model/model.h"
#include "core/sequencer.h"
#include "glue/channel.h"
#include "glue/events.h"
#include <cassert>

extern giada::m::Engine g_engine;

namespace giada::c::actionEditor
{
SampleData::SampleData(const m::SamplePlayer& s)
: channelMode(s.mode)
, isLoopMode(s.isAnyLoopMode())
{
}

/* -------------------------------------------------------------------------- */

Data::Data(const m::Channel& c)
: channelId(c.id)
, channelName(c.name)
, framesInSeq(g_engine.sequencer.getFramesInSeq())
, framesInBeat(g_engine.sequencer.getFramesInBeat())
, framesInBar(g_engine.sequencer.getFramesInBar())
, framesInLoop(g_engine.sequencer.getFramesInLoop())
, actions(g_engine.actionRecorder.getActionsOnChannel(c.id))
{
	if (c.type == ChannelType::SAMPLE)
		sample = std::make_optional<SampleData>(c.samplePlayer.value());
}

/* -------------------------------------------------------------------------- */

Frame Data::getCurrentFrame() const
{
	return g_engine.sequencer.getCurrentFrame();
}

/* -------------------------------------------------------------------------- */

bool Data::isChannelPlaying() const
{
	return g_engine.model.get().getChannel(channelId).isPlaying();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Data getData(ID channelId)
{
	return Data(g_engine.model.get().getChannel(channelId));
}

/* -------------------------------------------------------------------------- */

void recordMidiAction(ID channelId, int note, int velocity, Frame f1, Frame f2)
{
	g_engine.actionRecorder.recordMidiAction(channelId, note, velocity, f1, f2, g_engine.sequencer.getFramesInLoop());
}

/* -------------------------------------------------------------------------- */

void deleteMidiAction(ID channelId, const m::Action& a)
{
	/* Send a note-off first in case we are deleting it in a middle of a 
	key_on/key_off sequence. */

	events::sendMidiToChannel(channelId, a.next->event, Thread::MAIN);

	g_engine.actionRecorder.deleteMidiAction(channelId, a);
}

/* -------------------------------------------------------------------------- */

void updateMidiAction(ID channelId, const m::Action& a, int note, int velocity,
    Frame f1, Frame f2)
{
	g_engine.actionRecorder.updateMidiAction(channelId, a, note, velocity, f1, f2, g_engine.sequencer.getFramesInLoop());
}

/* -------------------------------------------------------------------------- */

void recordSampleAction(ID channelId, int type, Frame f1, Frame f2)
{
	g_engine.actionRecorder.recordSampleAction(channelId, type, f1, f2);
}

/* -------------------------------------------------------------------------- */

void updateSampleAction(ID channelId, const m::Action& a, int type, Frame f1, Frame f2)
{
	g_engine.actionRecorder.updateSampleAction(channelId, a, type, f1, f2);
}

/* -------------------------------------------------------------------------- */

void deleteSampleAction(ID channelId, const m::Action& a)
{
	g_engine.actionRecorder.deleteSampleAction(channelId, a);
}

/* -------------------------------------------------------------------------- */

void recordEnvelopeAction(ID channelId, Frame f, int value)
{
	g_engine.actionRecorder.recordEnvelopeAction(channelId, f, value, g_engine.sequencer.getFramesInLoop() - 1);
}

/* -------------------------------------------------------------------------- */

void deleteEnvelopeAction(ID channelId, const m::Action& a)
{
	g_engine.actionRecorder.deleteEnvelopeAction(channelId, a);
}

/* -------------------------------------------------------------------------- */

void updateEnvelopeAction(ID channelId, const m::Action& a, Frame f, int value)
{
	g_engine.actionRecorder.updateEnvelopeAction(channelId, a, f, value, g_engine.sequencer.getFramesInLoop() - 1);
}

/* -------------------------------------------------------------------------- */

void updateVelocity(const m::Action& a, int value)
{
	g_engine.actionRecorder.updateVelocity(a, value);
}
} // namespace giada::c::actionEditor
