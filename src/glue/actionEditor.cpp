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
#include "glue/events.h"
#include "glue/recorder.h"
#include <cassert>

extern giada::m::Engine g_engine;

namespace giada::c::actionEditor
{
namespace
{
Frame fixVerticalEnvActions_(Frame f, const m::Action& a1, const m::Action& a2)
{
	if (a1.frame == f)
		f += 1;
	else if (a2.frame == f)
		f -= 1;
	if (a1.frame == f || a2.frame == f)
		return -1;
	return f;
}

/* -------------------------------------------------------------------------- */

/* recordFirstEnvelopeAction_
First action ever? Add actions at boundaries. */

void recordFirstEnvelopeAction_(ID channelId, Frame frame, int value)
{
	// TODO - use MidiEvent(float)
	m::MidiEvent    e1 = m::MidiEvent(m::MidiEvent::CHANNEL_CC, 0, G_MAX_VELOCITY);
	m::MidiEvent    e2 = m::MidiEvent(m::MidiEvent::CHANNEL_CC, 0, value);
	const m::Action a1 = g_engine.actionRecorder.rec(channelId, 0, e1);
	const m::Action a2 = g_engine.actionRecorder.rec(channelId, frame, e2);
	const m::Action a3 = g_engine.actionRecorder.rec(channelId, g_engine.sequencer.getFramesInLoop() - 1, e1);

	g_engine.actionRecorder.updateSiblings(a1.id, /*prev=*/a3.id, /*next=*/a2.id); // Circular loop (begin)
	g_engine.actionRecorder.updateSiblings(a2.id, /*prev=*/a1.id, /*next=*/a3.id);
	g_engine.actionRecorder.updateSiblings(a3.id, /*prev=*/a2.id, /*next=*/a1.id); // Circular loop (end)
}

/* -------------------------------------------------------------------------- */

/* recordNonFirstEnvelopeAction_
Find action right before frame 'frame' and inject a new action in there. 
Vertical envelope points are forbidden. */

void recordNonFirstEnvelopeAction_(ID channelId, Frame frame, int value)
{
	const m::Action a1 = g_engine.actionRecorder.getClosestAction(channelId, frame, m::MidiEvent::CHANNEL_CC);
	const m::Action a3 = a1.next != nullptr ? *a1.next : m::Action{};

	assert(a1.isValid());
	assert(a3.isValid());

	frame = fixVerticalEnvActions_(frame, a1, a3);
	if (frame == -1) // Vertical points, nothing to do here
		return;

	// TODO - use MidiEvent(float)
	m::MidiEvent    e2 = m::MidiEvent(m::MidiEvent::CHANNEL_CC, 0, value);
	const m::Action a2 = g_engine.actionRecorder.rec(channelId, frame, e2);

	g_engine.actionRecorder.updateSiblings(a2.id, a1.id, a3.id);
}

/* -------------------------------------------------------------------------- */

bool isSinglePressMode_(ID channelId)
{
	/* TODO - use m::model getChannel utils (to be added) */
	return g_engine.model.get().getChannel(channelId).samplePlayer->mode == SamplePlayerMode::SINGLE_PRESS;
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

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
	if (f2 == 0)
		f2 = f1 + G_DEFAULT_ACTION_SIZE;

	/* Avoid frame overflow. */

	Frame overflow = f2 - (g_engine.sequencer.getFramesInLoop());
	if (overflow > 0)
	{
		f2 -= overflow;
		f1 -= overflow;
	}

	m::MidiEvent e1 = m::MidiEvent(m::MidiEvent::CHANNEL_NOTE_ON, note, velocity);
	m::MidiEvent e2 = m::MidiEvent(m::MidiEvent::CHANNEL_NOTE_OFF, note, velocity);

	g_engine.actionRecorder.rec(channelId, f1, f2, e1, e2);

	recorder::updateChannel(channelId, /*updateActionEditor=*/false);
}

/* -------------------------------------------------------------------------- */

void deleteMidiAction(ID channelId, const m::Action& a)
{
	assert(a.isValid());
	assert(a.event.getStatus() == m::MidiEvent::CHANNEL_NOTE_ON);

	/* Send a note-off first in case we are deleting it in a middle of a 
	key_on/key_off sequence. Check if 'next' exist first: could be orphaned. */

	if (a.next != nullptr)
	{
		events::sendMidiToChannel(channelId, a.next->event, Thread::MAIN);
		g_engine.actionRecorder.deleteAction(a.id, a.next->id);
	}
	else
		g_engine.actionRecorder.deleteAction(a.id);

	recorder::updateChannel(channelId, /*updateActionEditor=*/false);
}

/* -------------------------------------------------------------------------- */

void updateMidiAction(ID channelId, const m::Action& a, int note, int velocity,
    Frame f1, Frame f2)
{
	g_engine.actionRecorder.deleteAction(a.id, a.next->id);
	recordMidiAction(channelId, note, velocity, f1, f2);
}

/* -------------------------------------------------------------------------- */

void recordSampleAction(ID channelId, int type, Frame f1, Frame f2)
{
	if (isSinglePressMode_(channelId))
	{
		if (f2 == 0)
			f2 = f1 + G_DEFAULT_ACTION_SIZE;
		m::MidiEvent e1 = m::MidiEvent(m::MidiEvent::CHANNEL_NOTE_ON, 0, 0);
		m::MidiEvent e2 = m::MidiEvent(m::MidiEvent::CHANNEL_NOTE_OFF, 0, 0);
		g_engine.actionRecorder.rec(channelId, f1, f2, e1, e2);
	}
	else
	{
		m::MidiEvent e1 = m::MidiEvent(type, 0, 0);
		g_engine.actionRecorder.rec(channelId, f1, e1);
	}

	recorder::updateChannel(channelId, /*updateActionEditor=*/false);
}

/* -------------------------------------------------------------------------- */

void updateSampleAction(ID channelId, const m::Action& a, int type,
    Frame f1, Frame f2)
{
	if (isSinglePressMode_(channelId))
		g_engine.actionRecorder.deleteAction(a.id, a.next->id);
	else
		g_engine.actionRecorder.deleteAction(a.id);

	recordSampleAction(channelId, type, f1, f2);
}

/* -------------------------------------------------------------------------- */

void deleteSampleAction(ID channelId, const m::Action& a)
{
	if (a.next != nullptr) // For ChannelMode::SINGLE_PRESS combo
		g_engine.actionRecorder.deleteAction(a.id, a.next->id);
	else
		g_engine.actionRecorder.deleteAction(a.id);

	recorder::updateChannel(channelId, /*updateActionEditor=*/false);
}

/* -------------------------------------------------------------------------- */

void recordEnvelopeAction(ID channelId, Frame f, int value)
{
	assert(value >= 0 && value <= G_MAX_VELOCITY);

	/* First action ever? Add actions at boundaries. Else, find action right
	before frame 'f' and inject a new action in there. Vertical envelope points 
	are forbidden for now. */

	if (!g_engine.actionRecorder.hasActions(channelId, m::MidiEvent::CHANNEL_CC))
		recordFirstEnvelopeAction_(channelId, f, value);
	else
		recordNonFirstEnvelopeAction_(channelId, f, value);

	recorder::updateChannel(channelId, /*updateActionEditor=*/false);
}

/* -------------------------------------------------------------------------- */

void deleteEnvelopeAction(ID channelId, const m::Action& a)
{
	/* Deleting a boundary action wipes out everything. If is volume, remember 
	to restore _i and _d members in channel. */
	/* TODO - move this to c::*/
	/* TODO - FIX*/
	if (g_engine.actionRecorder.isBoundaryEnvelopeAction(a))
	{
		if (a.isVolumeEnvelope())
		{
			// TODO reset all volume vars to 1.0
		}
		g_engine.actionRecorder.clearActions(channelId, a.event.getStatus());
	}
	else
	{
		assert(a.prev != nullptr);
		assert(a.next != nullptr);

		const m::Action a1     = *a.prev;
		const m::Action a1prev = *a1.prev;
		const m::Action a3     = *a.next;
		const m::Action a3next = *a3.next;

		/* Original status:   a1--->a--->a3
		   Modified status:   a1-------->a3 
		Order is important, here: first update siblings, then delete the action.
		Otherwise ActionRecorder::deleteAction() would complain of missing 
		prevId/nextId no longer found. */

		g_engine.actionRecorder.updateSiblings(a1.id, a1prev.id, a3.id);
		g_engine.actionRecorder.updateSiblings(a3.id, a1.id, a3next.id);
		g_engine.actionRecorder.deleteAction(a.id);
	}

	recorder::updateChannel(channelId, /*updateActionEditor=*/false);
}

/* -------------------------------------------------------------------------- */

void updateEnvelopeAction(ID channelId, const m::Action& a, Frame f, int value)
{
	/* Update the action directly if it is a boundary one. Else, delete the
	previous one and record a new action. */

	if (g_engine.actionRecorder.isBoundaryEnvelopeAction(a))
		g_engine.actionRecorder.updateEvent(a.id, m::MidiEvent(m::MidiEvent::CHANNEL_CC, 0, value));
	else
	{
		deleteEnvelopeAction(channelId, a);
		recordEnvelopeAction(channelId, f, value);
	}
}

/* -------------------------------------------------------------------------- */

std::vector<m::Action> getActions(ID channelId)
{
	return g_engine.actionRecorder.getActionsOnChannel(channelId);
}

/* -------------------------------------------------------------------------- */

void updateVelocity(const m::Action& a, int value)
{
	m::MidiEvent event(a.event);
	event.setVelocity(value);

	g_engine.actionRecorder.updateEvent(a.id, event);
}
} // namespace giada::c::actionEditor
