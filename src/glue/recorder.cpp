/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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


#include "../gui/dialogs/gd_warnings.h"
#include "../gui/elems/mainWindow/keyboard/channel.h"
#include "../gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "../core/const.h"
#include "../core/clock.h"
#include "../core/kernelMidi.h"
#include "../core/channel.h"
#include "../core/recorder.h"
#include "../utils/gui.h"
#include "../utils/log.h"
#include "recorder.h"


using std::vector;
using namespace giada;


namespace giada {
namespace c     {
namespace recorder 
{
namespace
{
void updateChannel(geChannel* gch)
{
	gch->ch->hasActions = m::recorder::hasActions(gch->ch->index);
	if (gch->ch->type == CHANNEL_SAMPLE && !gch->ch->hasActions)
		static_cast<geSampleChannel*>(gch)->hideActionButton();
	/* TODO - set mute=false */
	gu_refreshActionEditor(); // refresh a.editor window, it could be open
}
}; // {namespace}


/* -------------------------------------------------------------------------- */


void clearAllActions(geChannel* gch)
{
	if (!gdConfirmWin("Warning", "Clear all actions: are you sure?"))
		return;
	m::recorder::clearChan(gch->ch->index);
	updateChannel(gch);
}


/* -------------------------------------------------------------------------- */


void clearVolumeActions(geChannel* gch)
{
	if (!gdConfirmWin("Warning", "Clear all volume actions: are you sure?"))
		return;
	m::recorder::clearAction(gch->ch->index, G_ACTION_VOLUME);
	updateChannel(gch);
}


/* -------------------------------------------------------------------------- */


void clearStartStopActions(geChannel* gch)
{
	if (!gdConfirmWin("Warning", "Clear all start/stop actions: are you sure?"))
		return;
	m::recorder::clearAction(gch->ch->index, G_ACTION_KEYPRESS | G_ACTION_KEYREL | G_ACTION_KILL);
	updateChannel(gch);
}


/* -------------------------------------------------------------------------- */


void clearMuteActions(geChannel* gch)
{
	if (!gdConfirmWin("Warning", "Clear all mute actions: are you sure?"))
		return;
	m::recorder::clearAction(gch->ch->index, G_ACTION_MUTEON | G_ACTION_MUTEOFF);
	updateChannel(gch);
}


/* -------------------------------------------------------------------------- */


void recordMidiAction(int chan, int note, int frame_a, int frame_b)
{
	if (frame_b == 0)
		frame_b = frame_a + G_DEFAULT_MIDI_ACTION_SIZE;

	/* Avoid frame overflow. */

	int overflow = frame_b - (m::clock::getFramesInLoop());
	if (overflow > 0) {
		frame_b -= overflow;
		frame_a -= overflow;
	}

	/* Prepare MIDI events, with maximum velocity (0x3F) for now. */

	m::MidiEvent event_a = m::MidiEvent(m::MidiEvent::NOTE_ON,  note, 0x3F);
	m::MidiEvent event_b = m::MidiEvent(m::MidiEvent::NOTE_OFF, note, 0x3F);

	/* Avoid overlapping actions. Find the next action past frame_a and compare 
	its frame: if smaller than frame_b, an overlap occurs. Shrink the new action
	accordingly. */

	m::recorder::action* next = nullptr;
	m::recorder::getNextAction(chan, G_ACTION_MIDI, frame_a, &next, event_a.getRaw(), 
		0x0000FF00);

	if (next != nullptr && next->frame <= frame_b) {
		frame_b = next->frame - 2;
		gu_log("[recorder::recordMidiAction] Shrink new action, due to overlap\n");
	}

	m::recorder::rec(chan, G_ACTION_MIDI, frame_a, event_a.getRaw());
	m::recorder::rec(chan, G_ACTION_MIDI, frame_b, event_b.getRaw());		
}


/* -------------------------------------------------------------------------- */


vector<m::recorder::Composite> getMidiActions(int chan, int frameLimit)
{
	vector<m::recorder::Composite> out;

	m::recorder::sortActions();

	for (unsigned i=0; i<m::recorder::frames.size(); i++) {

		if (m::recorder::frames.at(i) > frameLimit)
			continue;

		for (unsigned j=0; j<m::recorder::global.at(i).size(); j++) {

			m::recorder::action* a1 = m::recorder::global.at(i).at(j);
			m::recorder::action* a2 = nullptr;

			m::MidiEvent a1midi(a1->iValue);

			/* Skip action if:
				- does not belong to this channel
				- is not a MIDI action (we only want MIDI things here)
				- is not a MIDI Note On type. We don't want any other kind of action here */

			if (a1->chan != chan || a1->type != G_ACTION_MIDI || 
				  a1midi.getStatus() != m::MidiEvent::NOTE_ON)
				continue;

			/* Prepare the composite action. Action 1 exists for sure, so fill it up
			right away. */

			m::recorder::Composite cmp; 
			cmp.a1 = *a1;

			/* Search for the next action. Must have: same channel, G_ACTION_MIDI,
			greater than a1->frame and with MIDI properties of note_off (0x80), same
			note of a1 and random velocity: we don't care about it (and so we mask it
			with 0x0000FF00). */

			m::MidiEvent a2midi(m::MidiEvent::NOTE_OFF, a1midi.getNote(), 0x0);

			m::recorder::getNextAction(chan, G_ACTION_MIDI, a1->frame, &a2, 
				a2midi.getRaw(), 0x0000FF00);

			/* If action 2 has been found, add it to the composite duo. Otherwise
			set the action 2 frame to -1: it should be intended as "orphaned". */

			if (a2 != nullptr)
				cmp.a2 = *a2;
			else
				cmp.a2.frame = -1;

			out.push_back(cmp);
		}
	}

	return out;
}

}}} // giada::c::recorder::