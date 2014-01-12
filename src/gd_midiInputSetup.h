/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_midiInputSetup
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2014 Giovanni A. Zuliani | Monocasual
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


#ifndef GD_MIDI_INPUT_SETUP_H
#define GD_MIDI_INPUT_SETUP_H


#include <FL/Fl.H>
#include "ge_window.h"


class gdMidiInputSetup : public gWindow {

private:

	static void cb_save          (Fl_Widget *w, void *p);
	static void cb_cancel        (Fl_Widget *w, void *p);
	static void cb_enableChanList(Fl_Widget *w, void *p);
	inline void __cb_save          ();
	inline void __cb_cancel        ();
	inline void __cb_enableChanList();

	void fillChanMenu(class gChoice *m);

	class gCheck  *enableOut;
	class gChoice *chanListOut;
	class gButton *save;
	class gButton *cancel;

	class MidiChannel *ch;

public:

	gdMidiInputSetup(class MidiChannel *ch);
};

#endif
