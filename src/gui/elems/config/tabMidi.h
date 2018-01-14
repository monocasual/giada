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


#ifndef GE_TAB_MIDI_H
#define GE_TAB_MIDI_H


#include <FL/Fl_Group.H>


class geChoice;
class geCheck;


class geTabMidi : public Fl_Group
{
private:

	void fetchSystems();
	void fetchOutPorts();
	void fetchInPorts();
	void fetchMidiMaps();

	static void cb_changeSystem  (Fl_Widget *w, void *p);
	inline void __cb_changeSystem();

	int systemInitValue;

public:

	geChoice *system;
	geChoice *portOut;
	geChoice *portIn;
	geCheck  *noNoteOff;
	geChoice *midiMap;
	geChoice *sync;

	geTabMidi(int x, int y, int w, int h);

	void save();
};


#endif
