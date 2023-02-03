/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "deps/geompp/src/rect.hpp"
#include "glue/config.h"
#include <FL/Fl_Group.H>

class geCheck;

namespace giada::v
{
class geStringMenu;
class geChoice;
class geTabMidi : public Fl_Group
{
public:
	geTabMidi(geompp::Rect<int>);

	void save() const;

	geChoice*     system;
	geStringMenu* portOut;
	geStringMenu* portIn;
	geCheck*      enableOut;
	geCheck*      enableIn;
	geStringMenu* midiMap;
	geChoice*     sync;

private:
	void invalidate();

	c::config::MidiData m_data;

	int m_initialApi;
};
} // namespace giada::v

#endif
