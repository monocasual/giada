/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_midiSetup
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


#include "gd_midiSetup.h"
#include "conf.h"
#include "ge_mixed.h"
#include "gg_keyboard.h"
#include "channel.h"
#include "gui_utils.h"


extern Conf	G_Conf;


gdMidiSetup::gdMidiSetup(MidiChannel *ch)
	: Fl_Window(300, 64, "Midi Output Setup"), ch(ch)
{
	begin();
	enableOut      = new gCheck(x()+8, y()+8, 150, 20, "Enable MIDI output");
	chanListOut    = new gChoice(w()-108, y()+8, 100, 20);

	save   = new gButton(w()-88, chanListOut->y()+chanListOut->h()+8, 80, 20, "Save");
	cancel = new gButton(w()-88-save->w()-8, save->y(), 80, 20, "Cancel");
	end();

	fillChanMenu(chanListOut);

	if (ch->midiOut)
		enableOut->value(1);
	else
		chanListOut->deactivate();

	chanListOut->value(ch->midiOutChan);

	enableOut->callback(cb_enableChanList, (void*)this);
	save->callback(cb_save, (void*)this);
	cancel->callback(cb_cancel, (void*)this);

	set_modal();
	gu_setFavicon(this);
	show();
}


/* ------------------------------------------------------------------ */


void gdMidiSetup::cb_save          (Fl_Widget *w, void *p) { ((gdMidiSetup*)p)->__cb_save(); }
void gdMidiSetup::cb_cancel        (Fl_Widget *w, void *p) { ((gdMidiSetup*)p)->__cb_cancel(); }
void gdMidiSetup::cb_enableChanList(Fl_Widget *w, void *p) { ((gdMidiSetup*)p)->__cb_enableChanList(); }


/* ------------------------------------------------------------------ */


void gdMidiSetup::__cb_enableChanList() {
	enableOut->value() ? chanListOut->activate() : chanListOut->deactivate();
}


/* ------------------------------------------------------------------ */


void gdMidiSetup::__cb_save() {
	ch->midiOut     = enableOut->value();
	ch->midiOutChan = chanListOut->value();
	ch->guiChannel->update();
	do_callback();
}


/* ------------------------------------------------------------------ */


void gdMidiSetup::__cb_cancel() { do_callback(); }


/* ------------------------------------------------------------------ */


void gdMidiSetup::fillChanMenu(gChoice *m) {
	m->add("Channel 1");
	m->add("Channel 2");
	m->add("Channel 3");
	m->add("Channel 4");
	m->add("Channel 5");
	m->add("Channel 6");
	m->add("Channel 7");
	m->add("Channel 8");
	m->add("Channel 9");
	m->add("Channel 10");
	m->add("Channel 11");
	m->add("Channel 12");
	m->add("Channel 13");
	m->add("Channel 14");
	m->add("Channel 15");
	m->add("Channel 16");
	m->value(0);
}
