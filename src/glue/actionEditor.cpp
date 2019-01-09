/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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
#include "../core/clock.h"
#include "../core/const.h"
#include "../core/sampleChannel.h"
#include "../core/midiChannel.h"
#include "../core/recorderHandler.h"
#include "../core/recorder.h"
#include "../core/action.h"
#include "recorder.h"
#include "actionEditor.h"


using std::vector;


namespace giada {
namespace c {
namespace actionEditor
{
namespace
{
Frame fixVerticalEnvActions_(Frame f, const m::Action* a1, const m::Action* a2)
{
	if      (a1->frame == f) f += 1;
	else if (a2->frame == f) f -= 1;
	if (a1->frame == f || a2->frame == f)
		return -1;
	return f;
}


/* -------------------------------------------------------------------------- */

/* recordFirstEnvelopeAction_
First action ever? Add actions at boundaries. */

void recordFirstEnvelopeAction_(int channel, Frame frame, int value)
{
	namespace mr = m::recorder;

	m::MidiEvent e1 = m::MidiEvent(m::MidiEvent::ENVELOPE, 0, G_MAX_VELOCITY);
	m::MidiEvent e2 = m::MidiEvent(m::MidiEvent::ENVELOPE, 0, value);
	const m::Action* a1 = mr::rec(channel, 0, e1);	
	const m::Action* a2 = mr::rec(channel, frame, e2);
	const m::Action* a3 = mr::rec(channel, m::clock::getFramesInLoop() - 1, e1);
	mr::updateSiblings(a1, a3, a2); // Circular loop (begin)
	mr::updateSiblings(a2, a1, a3);
	mr::updateSiblings(a3, a2, a1); // Circular loop (end)
}


/* -------------------------------------------------------------------------- */


/* recordNonFirstEnvelopeAction_
Find action right before frame 'frame' and inject a new action in there. 
Vertical envelope points are forbidden. */

void recordNonFirstEnvelopeAction_(int channel, Frame frame, int value)
{
	namespace mr = m::recorder;

	m::MidiEvent e2 = m::MidiEvent(m::MidiEvent::ENVELOPE, 0, value);
	const m::Action* a1 = mr::getClosestAction(channel, frame, m::MidiEvent::ENVELOPE);
	const m::Action* a3 = a1->next;
	assert(a1 != nullptr);
	assert(a3 != nullptr);
	frame = fixVerticalEnvActions_(frame, a1, a3);
	if (frame == -1) // Vertical points, nothing to do here
		return;
	const m::Action* a2 = mr::rec(channel, frame, e2);
	mr::updateSiblings(a2, a1, a3);
}
}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void recordMidiAction(m::MidiChannel* ch, int note, int velocity, Frame f1, Frame f2)
{
	namespace mr = m::recorder;
	namespace cr = c::recorder;

	if (f2 == 0)
		f2 = f1 + G_DEFAULT_ACTION_SIZE;

	/* Avoid frame overflow. */

	int overflow = f2 - (m::clock::getFramesInLoop());
	if (overflow > 0) {
		f2 -= overflow;
		f1 -= overflow;
	}

	m::MidiEvent e1 = m::MidiEvent(m::MidiEvent::NOTE_ON,  note, velocity);
	m::MidiEvent e2 = m::MidiEvent(m::MidiEvent::NOTE_OFF, note, velocity);

	const m::Action* a1 = mr::rec(ch->index, f1, e1);
	const m::Action* a2 = mr::rec(ch->index, f2, e2);

	mr::updateSiblings(a1, nullptr, a2);

	cr::updateChannel(ch->guiChannel, /*refreshActionEditor=*/false);
}


/* -------------------------------------------------------------------------- */


void deleteMidiAction(m::MidiChannel* ch, const m::Action* a)
{
	namespace mr = m::recorder;
	namespace cr = c::recorder;

	assert(a != nullptr);
	assert(a->event.getStatus() == m::MidiEvent::NOTE_ON);

	/* Send a note-off first in case we are deleting it in a middle of a 
	key_on/key_off sequence. */
	
	if (a->next != nullptr) {
		ch->sendMidi(a->next, 0);
		mr::deleteAction(a->next);
	}
	mr::deleteAction(a);

	cr::updateChannel(ch->guiChannel, /*refreshActionEditor=*/false);
}

/* -------------------------------------------------------------------------- */


void updateMidiAction(m::MidiChannel* ch, const m::Action* a, int note, int velocity, 
	Frame f1, Frame f2)
{
	namespace mr = m::recorder;

	mr::deleteAction(a->next);
	mr::deleteAction(a);
	
	recordMidiAction(ch, note, velocity, f1, f2);
}


/* -------------------------------------------------------------------------- */


void recordSampleAction(const m::SampleChannel* ch, int type, Frame f1, Frame f2)
{
	namespace mr = m::recorder;
	namespace cr = c::recorder;

	if (ch->mode == ChannelMode::SINGLE_PRESS) {
		m::MidiEvent e1 = m::MidiEvent(m::MidiEvent::NOTE_ON, 0, 0);
		m::MidiEvent e2 = m::MidiEvent(m::MidiEvent::NOTE_OFF, 0, 0);
		const m::Action* a1 = mr::rec(ch->index, f1, e1);
		const m::Action* a2 = mr::rec(ch->index, f2 == 0 ? f1 + G_DEFAULT_ACTION_SIZE : f2, e2);
		mr::updateSiblings(a1, nullptr, a2);
	}
	else {
		m::MidiEvent e1 = m::MidiEvent(type, 0, 0);
		mr::rec(ch->index, f1, e1);
	}
	
	cr::updateChannel(ch->guiChannel, /*refreshActionEditor=*/false);
}


/* -------------------------------------------------------------------------- */


void updateSampleAction(m::SampleChannel* ch, const m::Action* a, int type, Frame f1, 
	Frame f2)
{
	namespace mr = m::recorder;	

	if (ch->mode == ChannelMode::SINGLE_PRESS)
		mr::deleteAction(a->next);
	mr::deleteAction(a);

	recordSampleAction(ch, type, f1, f2);
}


/* -------------------------------------------------------------------------- */


void recordEnvelopeAction(m::Channel* ch, int frame, int value)
{
	namespace mr = m::recorder;	
	namespace cr = c::recorder;	

	assert(value >= 0 && value <= G_MAX_VELOCITY);

	/* First action ever? Add actions at boundaries. Else, find action right
	before frame 'f' and inject a new action in there. Vertical envelope points 
	are forbidden for now. */

	if (!mr::hasActions(ch->index, m::MidiEvent::ENVELOPE))
		recordFirstEnvelopeAction_(ch->index, frame, value);
	else 
		recordNonFirstEnvelopeAction_(ch->index, frame, value);

	cr::updateChannel(ch->guiChannel, /*refreshActionEditor=*/false);
}


/* -------------------------------------------------------------------------- */


void deleteEnvelopeAction(m::Channel* ch, const m::Action* a)
{
	namespace mr  = m::recorder;
	namespace cr  = c::recorder;
	namespace mrh = m::recorderHandler;

	assert(a != nullptr);

	/* Delete a boundary action wipes out everything. If is volume, remember to
	restore _i and _d members in channel. */

	if (mrh::isBoundaryEnvelopeAction(a)) {
		if (a->isVolumeEnvelope()) {
			ch->volume_i = 1.0;
			ch->volume_d = 0.0;
		}
		mr::clearActions(ch->index, a->event.getStatus());
		return;
	}

	const m::Action* a1 = a->prev;
	const m::Action* a3 = a->next; 

	/* Original status:   a1--->a--->a3
	   Modified status:   a1-------->a3 */

	mr::deleteAction(a);
	mr::updateSiblings(a1, a1->prev, a3);
	mr::updateSiblings(a3, a1, a3->next);

	cr::updateChannel(ch->guiChannel, /*refreshActionEditor=*/false);
}


/* -------------------------------------------------------------------------- */


void updateEnvelopeAction(m::Channel* ch, const m::Action* a, int frame, int value)
{
	namespace mr  = m::recorder;
	namespace cr  = c::recorder;
	namespace mrh = m::recorderHandler;

	assert(a != nullptr);

	/* Update the action directly if it is a boundary one. Else, delete the
	previous one and record a new action. */

	if (mrh::isBoundaryEnvelopeAction(a))
		mr::updateEvent(a, m::MidiEvent(m::MidiEvent::ENVELOPE, 0, value));
	else {
		deleteEnvelopeAction(ch, a);
		recordEnvelopeAction(ch, frame, value); 
	}

	cr::updateChannel(ch->guiChannel, /*refreshActionEditor=*/false);	
}


/* -------------------------------------------------------------------------- */


void deleteSampleAction(m::SampleChannel* ch, const m::Action* a)
{
	namespace mr = m::recorder;
	namespace cr = c::recorder;

	assert(a != nullptr);

	if (a->next != nullptr) // For ChannelMode::SINGLE_PRESS combo
		mr::deleteAction(a->next);
	mr::deleteAction(a);

	cr::updateChannel(ch->guiChannel, /*refreshActionEditor=*/false);
}


/* -------------------------------------------------------------------------- */


vector<const m::Action*> getActions(const m::Channel* ch)
{
	return m::recorder::getActionsOnChannel(ch->index);
}


/* -------------------------------------------------------------------------- */


void updateVelocity(const m::MidiChannel* ch, const m::Action* a, int value)
{
	namespace mr = m::recorder;
	
	m::MidiEvent event(a->event);
	event.setVelocity(value);

	mr::updateEvent(a, event);
}
}}}; // giada::c::actionEditor::
