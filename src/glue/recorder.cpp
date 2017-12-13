/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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
#include "../core/kernelMidi.h"
#include "../core/channel.h"
#include "../core/recorder.h"
#include "../utils/gui.h"
#include "recorder.h"


using std::vector;
using namespace giada::m;


namespace
{
void updateChannel(geChannel *gch)
{
	gch->ch->hasActions = recorder::hasActions(gch->ch->index);
	if (gch->ch->type == CHANNEL_SAMPLE && !gch->ch->hasActions)
		static_cast<geSampleChannel*>(gch)->hideActionButton();
	/* TODO - set mute=false */
	gu_refreshActionEditor(); // refresh a.editor window, it could be open
}
}; // {namespace}


/* -------------------------------------------------------------------------- */


void glue_clearAllActions(geChannel *gch)
{
	if (!gdConfirmWin("Warning", "Clear all actions: are you sure?"))
		return;
	recorder::clearChan(gch->ch->index);
	updateChannel(gch);
}


/* -------------------------------------------------------------------------- */


void glue_clearVolumeActions(geChannel *gch)
{
	if (!gdConfirmWin("Warning", "Clear all volume actions: are you sure?"))
		return;
	recorder::clearAction(gch->ch->index, G_ACTION_VOLUME);
	updateChannel(gch);
}


/* -------------------------------------------------------------------------- */


void glue_clearStartStopActions(geChannel *gch)
{
	if (!gdConfirmWin("Warning", "Clear all start/stop actions: are you sure?"))
		return;
	recorder::clearAction(gch->ch->index, G_ACTION_KEYPRESS | G_ACTION_KEYREL | G_ACTION_KILL);
	updateChannel(gch);
}


/* -------------------------------------------------------------------------- */


void glue_clearMuteActions(geChannel *gch)
{
	if (!gdConfirmWin("Warning", "Clear all mute actions: are you sure?"))
		return;
	recorder::clearAction(gch->ch->index, G_ACTION_MUTEON | G_ACTION_MUTEOFF);
	updateChannel(gch);
}


/* -------------------------------------------------------------------------- */


vector<recorder::Composite> glue_getMidiActions(int chan, int frameLimit)
{
	vector<recorder::Composite> out;

	recorder::sortActions();

	for (unsigned i=0; i<recorder::frames.size(); i++) {

		if (recorder::frames.at(i) > frameLimit)
			continue;

		for (unsigned j=0; j<recorder::global.at(i).size(); j++) {

			recorder::action* a1 = recorder::global.at(i).at(j);
			recorder::action* a2 = nullptr;

			MidiEvent a1midi(a1->iValue);

			/* Skip action if:
				- does not belong to this channel
				- is not a MIDI action (we only want MIDI things here)
				- is not a MIDI Note On type. We don't want any other kind of action here */

			if (a1->chan != chan || a1->type != G_ACTION_MIDI || a1midi.getStatus() != MidiEvent::NOTE_ON)
				continue;

			/* Prepare the composite action. Action 1 exists for sure, so fill it up
			right away. */

			recorder::Composite cmp; 
			cmp.a1 = *a1;

			/* Search for the next action. Must have: same channel, G_ACTION_MIDI,
			greater than a1->frame and with MIDI properties of note_off (0x80), same
			note of a1 and random velocity: we don't care about it (and so we mask it
			with 0x0000FF00). */

			MidiEvent a2midi(MidiEvent::NOTE_OFF, a1midi.getNote(), 0x0);

			recorder::getNextAction(chan, G_ACTION_MIDI, a1->frame, &a2, 
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