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
	:	gWindow(300, 150, "MIDI input setup"),
		ch(ch)
{
	set_modal();

	new gLearn(8,  30, w()-16, "key press/release", ch, cb_learnKeyPressRel);
	new gLearn(8,  54, w()-16, "key kill", ch, cb_learnKeyPressRel);
	new gLearn(8,  78, w()-16, "mute", ch, cb_learnKeyPressRel);
	new gLearn(8, 102, w()-16, "solo", ch, cb_learnKeyPressRel);
	new gLearn(8, 126, w()-16, "volume", ch, cb_learnKeyPressRel);

	gu_setFavicon(this);
	show();
}


/* ------------------------------------------------------------------ */


gdMidiGrabber::~gdMidiGrabber() {
	kernelMidi::delMidiLearnCb();
}


/* ------------------------------------------------------------------ */


void gdMidiGrabber::cb_learnKeyPressRel(uint32_t msg, void *data) { ((gdMidiGrabber*)data)->__cb_learnKeyPressRel(msg); }

void gdMidiGrabber::__cb_learnKeyPressRel(uint32_t msg) {

	printf("[gdMidiGrabber] MIDI learn - message=0x%X\n", msg);

	ch->midiInKeyPress = msg;
	ch->midiInKeyRel   = msg & 0x8FFFFFFF;

	kernelMidi::delMidiLearnCb();  // learn done, remove callback

	printf("[gdMidiGrabber] MIDI learn - done\n");
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gLearn::gLearn(int X, int Y, int W, const char *l, class Channel *ch, kernelMidi::cb_midiLearn *cb)
	: Fl_Group(X, Y, W, 20),
		ch      (ch),
		callback(cb)
{
	begin();
	text   = new gBox(x(), y(), w()-44, 20, l);
	button = new gButton(x()+w()-40, y(), 40, 20, "learn");
	end();

	text->box(G_BOX);
	text->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	button->callback(cb_button, (void*)this);
}


/* ------------------------------------------------------------------ */


void gLearn::cb_button(Fl_Widget *v, void *p) { ((gLearn*)p)->__cb_button(); }


/* ------------------------------------------------------------------ */


void gLearn::__cb_button() {
	puts("learn on");
	kernelMidi::addMidiLearnCb(callback, (void*)this);
}
