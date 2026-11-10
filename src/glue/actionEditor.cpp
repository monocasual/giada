/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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
#include "src/core/model/modelView.h"
#include "src/core/model_DEPR_/model.h"
#include "src/core/sequencer.h"
#include "src/glue/channel.h"
#include "src/glue/main.h"
#include <cassert>

extern giada::m::Engine* g_engine;

namespace giada::c::actionEditor
{
Action::Action(const m::model::Action& a)
: id(a.id)
, tick(a.tick)
, event(a.event)
, prevId(a.prevId)
, nextId(a.nextId)
, isValid(a.isValid())
, isVolumeEnvelope(a.isVolumeEnvelope())
{
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

SampleData::SampleData(const m::model::SampleChannel& s)
: channelMode(s.mode)
, isLoopMode(s.isAnyLoopMode())
{
}

/* -------------------------------------------------------------------------- */

Data::Data(const m::model::RealtimeModel::Snapshot& snapshot, ID channelId)
: channelId(channelId)
, scene(snapshot.getParameters().sequencer.getCurrentScene())
, channelName(snapshot.getDocument().tracks.getChannel(channelId).getName(scene))
, isChannelPlaying(snapshot.getAssets().channelsParameters.get(channelId)->isPlaying())
, ticksInSeq(snapshot.getDocument().sequencer.getTicksInSeq())
, ticksInBeat(snapshot.getDocument().sequencer.getTicksInBeat())
, ticksInBar(snapshot.getDocument().sequencer.getTicksInBar())
, ticksInLoop(snapshot.getDocument().sequencer.getTicksInLoop())
, m_snapshot(snapshot)
{
	const auto view = m::model::makeModelView(m_snapshot);
	const m::model::Channel& ch = view.getDocument().tracks.getChannel(channelId);
	if (ch.type == ChannelType::SAMPLE)
		sample = std::make_optional<SampleData>(ch.sampleChannel.value());

	for (const m::model::Action* action : view.getDocument().actions.getActionsOnChannel(channelId, scene))
		actions.push_back(Action(*action));
}

/* -------------------------------------------------------------------------- */

Tick Data::getCurrentTick() const
{
	const auto view = m::model::makeModelView(m_snapshot);
	return view.getParameters().sequencer.getCurrentTick(
	    view.getDocument().kernelAudio.samplerate,
	    view.getDocument().sequencer.getBpm());
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Data getData(ID channelId)
{
	const auto snapshot = g_engine->getModelSnapshot();
	return Data(snapshot, channelId);
}

/* -------------------------------------------------------------------------- */

std::optional<Action> findAction(ID id)
{
	const m::model::Action* a = g_engine->getActionEditorApi().findAction(id);
	if (a != nullptr)
		return *a;
	return {};
}

/* -------------------------------------------------------------------------- */

void recordMidiAction(ID channelId, int note, float velocity, TickRange range)
{
	g_engine->getActionEditorApi().recordMidiAction(channelId, note, velocity, range);
}

/* -------------------------------------------------------------------------- */

void deleteMidiAction(ID channelId, ID actionId)
{
	g_engine->getActionEditorApi().deleteMidiAction(channelId, actionId);
}

/* -------------------------------------------------------------------------- */

void updateMidiAction(ID channelId, ID actionId, int note, float velocity,
    TickRange range)
{
	g_engine->getActionEditorApi().updateMidiAction(channelId, actionId, note, velocity, range);
}

/* -------------------------------------------------------------------------- */

void recordSampleAction(ID channelId, int type, Tick tick)
{
	g_engine->getActionEditorApi().recordSampleAction(channelId, type, tick);
}

/* -------------------------------------------------------------------------- */

void updateSampleAction(ID channelId, ID actionId, int type, Tick tick)
{
	g_engine->getActionEditorApi().updateSampleAction(channelId, actionId, type, tick);
}

/* -------------------------------------------------------------------------- */

void deleteSampleAction(ID actionId)
{
	g_engine->getActionEditorApi().deleteSampleAction(actionId);
}

/* -------------------------------------------------------------------------- */

void updateVelocity(ID actionId, float value)
{
	g_engine->getActionEditorApi().updateVelocity(actionId, value);
}
} // namespace giada::c::actionEditor
