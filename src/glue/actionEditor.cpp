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
, framesInSeq(g_engine.getMainEngine().getFramesInSeq())
, framesInBeat(g_engine.getMainEngine().getFramesInBeat())
, framesInBar(g_engine.getMainEngine().getFramesInBar())
, framesInLoop(g_engine.getMainEngine().getFramesInLoop())
, actions(g_engine.getActionEditorEngine().getActionsOnChannel(c.id))
{
	if (c.type == ChannelType::SAMPLE)
		sample = std::make_optional<SampleData>(c.samplePlayer.value());
}

/* -------------------------------------------------------------------------- */

Frame Data::getCurrentFrame() const
{
	return g_engine.getMainEngine().getCurrentFrame();
}

/* -------------------------------------------------------------------------- */

bool Data::isChannelPlaying() const
{
	return g_engine.getChannelsEngine().get(channelId).isPlaying();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Data getData(ID channelId)
{
	return Data(g_engine.getChannelsEngine().get(channelId));
}

/* -------------------------------------------------------------------------- */

void recordMidiAction(ID channelId, int note, int velocity, Frame f1, Frame f2)
{
	g_engine.getActionEditorEngine().recordMidiAction(channelId, note, velocity, f1, f2);
}

/* -------------------------------------------------------------------------- */

void deleteMidiAction(ID channelId, const m::Action& a)
{
	g_engine.getActionEditorEngine().deleteMidiAction(channelId, a);
}

/* -------------------------------------------------------------------------- */

void updateMidiAction(ID channelId, const m::Action& a, int note, int velocity,
    Frame f1, Frame f2)
{
	g_engine.getActionEditorEngine().updateMidiAction(channelId, a, note, velocity, f1, f2);
}

/* -------------------------------------------------------------------------- */

void recordSampleAction(ID channelId, int type, Frame f1, Frame f2)
{
	g_engine.getActionEditorEngine().recordSampleAction(channelId, type, f1, f2);
}

/* -------------------------------------------------------------------------- */

void updateSampleAction(ID channelId, const m::Action& a, int type, Frame f1, Frame f2)
{
	g_engine.getActionEditorEngine().updateSampleAction(channelId, a, type, f1, f2);
}

/* -------------------------------------------------------------------------- */

void deleteSampleAction(ID channelId, const m::Action& a)
{
	g_engine.getActionEditorEngine().deleteSampleAction(channelId, a);
}

/* -------------------------------------------------------------------------- */

void recordEnvelopeAction(ID channelId, Frame f, int value)
{
	g_engine.getActionEditorEngine().recordEnvelopeAction(channelId, f, value);
}

/* -------------------------------------------------------------------------- */

void deleteEnvelopeAction(ID channelId, const m::Action& a)
{
	g_engine.getActionEditorEngine().deleteEnvelopeAction(channelId, a);
}

/* -------------------------------------------------------------------------- */

void updateEnvelopeAction(ID channelId, const m::Action& a, Frame f, int value)
{
	g_engine.getActionEditorEngine().updateEnvelopeAction(channelId, a, f, value);
}

/* -------------------------------------------------------------------------- */

void updateVelocity(const m::Action& a, int value)
{
	g_engine.getActionEditorEngine().updateVelocity(a, value);
}
} // namespace giada::c::actionEditor
