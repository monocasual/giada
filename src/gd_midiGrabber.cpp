/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_midiGrabber
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013 Giovanni A. Zuliani | Monocasual
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
 * ------------------------------------------------------------------ */


#include "gd_midiGrabber.h"
#include "ge_mixed.h"
#include "gui_utils.h"
#include "kernelMidi.h"


gdMidiGrabber::gdMidiGrabber(Channel *ch)
	:	gWindow(300, 100, "MIDI input setup"),
		ch(ch)
{
	set_modal();
	text = new gBox(10, 10, 280, 80, "[msg here...]");
	kernelMidi::addMidiLearnCb(cb_midiLearn, (void*)this);
	gu_setFavicon(this);
	show();
}


/* ------------------------------------------------------------------ */


gdMidiGrabber::~gdMidiGrabber() {
	kernelMidi::delMidiLearnCb();
}


/* ------------------------------------------------------------------ */


void gdMidiGrabber::cb_midiLearn(void *data) {
	((gdMidiGrabber*)data)->__cb_midiLearn();
}


/* ------------------------------------------------------------------ */


void gdMidiGrabber::__cb_midiLearn() {
	text->label("ok");

	/** TODO - store channel::midiIn[...] */
}


