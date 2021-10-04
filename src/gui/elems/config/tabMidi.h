/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#include "glue/config.h"
#include "gui/elems/basics/choice.h"
#include <FL/Fl_Group.H>

class geCheck;

namespace giada::v
{
class geTabMidi : public Fl_Group
{
public:
	struct geMenu : public geChoice
	{
		geMenu(int x, int y, int w, int h, const char* l, const std::vector<std::string>&,
		    const std::string& msgIfNotFound);
	};

	geTabMidi(int x, int y, int w, int h);

	void save() const;

	geChoice* system;
	geMenu*   portOut;
	geMenu*   portIn;
	geCheck*  enableOut;
	geCheck*  enableIn;
	geMenu*   midiMap;
	geChoice* sync;

private:
	void invalidate();

	c::config::MidiData m_data;

	int m_initialApi;
};
} // namespace giada::v

#endif
