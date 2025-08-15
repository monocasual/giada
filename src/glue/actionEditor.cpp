/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/glue/actionEditor.h"
#include "src/core/engine.h"
#include "src/core/model/model.h"
#include "src/core/sequencer.h"
#include "src/glue/channel.h"
#include "src/glue/main.h"
#include <cassert>

extern giada::m::Engine* g_engine;

namespace giada::c::actionEditor
{
SampleData::SampleData(const m::SampleChannel& s)
: channelMode(s.mode)
, isLoopMode(s.isAnyLoopMode())
{
}

/* -------------------------------------------------------------------------- */

Data::Data(const m::Channel& c)
: channelId(c.id)
, channelName(c.getName())
, framesInSeq(g_engine->getMainApi().getFramesInSeq())
, framesInBeat(g_engine->getMainApi().getFramesInBeat())
, framesInBar(g_engine->getMainApi().getFramesInBar())
, framesInLoop(g_engine->getMainApi().getFramesInLoop())
, actions(g_engine->getActionEditorApi().getActionsOnChannel(c.id))
{
	if (c.type == ChannelType::SAMPLE)
		sample = std::make_optional<SampleData>(c.sampleChannel.value());
}

/* -------------------------------------------------------------------------- */

Frame Data::getCurrentFrame() const
{
	return g_engine->getMainApi().getCurrentFrame();
}

/* -------------------------------------------------------------------------- */

bool Data::isChannelPlaying() const
{
	return g_engine->getChannelsApi().get(channelId).isPlaying();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Data getData(ID channelId)
{
	return Data(g_engine->getChannelsApi().get(channelId));
}

/* -------------------------------------------------------------------------- */

const m::Action* findAction(ID id)
{
	return g_engine->getActionEditorApi().findAction(id);
}

/* -------------------------------------------------------------------------- */

void recordMidiAction(ID channelId, int note, float velocity, Frame f1, Frame f2)
{
	g_engine->getActionEditorApi().recordMidiAction(channelId, note, velocity, f1, f2);
}

/* -------------------------------------------------------------------------- */

void deleteMidiAction(ID channelId, const m::Action& a)
{
	g_engine->getActionEditorApi().deleteMidiAction(channelId, a);
}

/* -------------------------------------------------------------------------- */

void updateMidiAction(ID channelId, const m::Action& a, int note, float velocity,
    Frame f1, Frame f2)
{
	g_engine->getActionEditorApi().updateMidiAction(channelId, a, note, velocity, f1, f2);
}

/* -------------------------------------------------------------------------- */

void recordSampleAction(ID channelId, int type, Frame f1, Frame f2)
{
	g_engine->getActionEditorApi().recordSampleAction(channelId, type, f1, f2);
}

/* -------------------------------------------------------------------------- */

void updateSampleAction(ID channelId, const m::Action& a, int type, Frame f1, Frame f2)
{
	g_engine->getActionEditorApi().updateSampleAction(channelId, a, type, f1, f2);
}

/* -------------------------------------------------------------------------- */

void deleteSampleAction(ID channelId, const m::Action& a)
{
	g_engine->getActionEditorApi().deleteSampleAction(channelId, a);
}

/* -------------------------------------------------------------------------- */

#if 0

void recordEnvelopeAction(ID channelId, Frame f, int value)
{
	g_engine->getActionEditorApi().recordEnvelopeAction(channelId, f, value);
}

/* -------------------------------------------------------------------------- */

void deleteEnvelopeAction(ID channelId, const m::Action& a)
{
	g_engine->getActionEditorApi().deleteEnvelopeAction(channelId, a);
}

/* -------------------------------------------------------------------------- */

void updateEnvelopeAction(ID channelId, const m::Action& a, Frame f, int value)
{
	g_engine->getActionEditorApi().updateEnvelopeAction(channelId, a, f, value);
}

#endif

/* -------------------------------------------------------------------------- */

void updateVelocity(const m::Action& a, float value)
{
	g_engine->getActionEditorApi().updateVelocity(a, value);
}
} // namespace giada::c::actionEditor
