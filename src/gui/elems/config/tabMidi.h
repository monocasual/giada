/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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
#include "gui/elems/basics/table.h"
#include <FL/Fl_Group.H>

namespace giada::v
{
class geStringMenu;
class geChoice;
class geTabMidi : public Fl_Group
{
public:
	geTabMidi(geompp::Rect<int>);

private:
	class geDevices : public geTable
	{
	public:
		geDevices(const std::vector<m::KernelMidi::DeviceInfo>&, c::config::DeviceType);

		Fl_Widget*  setCellContent(int row, int col, int x, int y, int w, int h) override;
		std::string setHeaderText(int col) override;

	private:
		static constexpr int PADDING = 2;

		const std::vector<m::KernelMidi::DeviceInfo>& m_devices;
		c::config::DeviceType                         m_type;
	};

	void rebuild(const c::config::MidiData&);

	c::config::MidiData m_data;

	geChoice*     m_system;
	geDevices*    m_devicesOut;
	geDevices*    m_devicesIn;
	geStringMenu* m_midiMap;
	geChoice*     m_sync;
};
} // namespace giada::v

#endif
