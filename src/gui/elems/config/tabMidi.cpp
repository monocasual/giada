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

#include "gui/elems/config/tabMidi.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/choice.h"
#include "gui/elems/config/stringMenu.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include <string>

extern giada::v::Ui* g_ui;

namespace giada::v
{
geTabMidi::geDevices::geDevices(const std::vector<m::KernelMidi::DeviceInfo>& devices, c::config::DeviceType type)
: geTable()
, m_devices(devices)
, m_type(type)
{
	prepareLayout();
}

Fl_Widget* geTabMidi::geDevices::setCellContent(int row, int col, int X, int Y, int W, int H)
{
	assert(static_cast<std::size_t>(row) < m_devices.size());

	geFlex* container = nullptr;

	if (col == 0)
	{
		container = new geFlex(X, Y, W, H, Direction::HORIZONTAL);
		{
			container->addWidget(new geBox(), PADDING);
			container->addWidget(new geBox(X, Y, W, H, m_devices[row].name.c_str(), FL_ALIGN_LEFT));
			container->end();
		}
	}
	else
	{
		container = new geFlex(X, Y, W, H, Direction::HORIZONTAL, /*gutter=*/0, /*pad=*/{PADDING});
		{
			geCheck* check = new geCheck(0, 0, 0, 0);
			container->addWidget(new geBox());
			container->addWidget(check, geCheck::CHECKBOX_WIDTH);
			container->addWidget(new geBox());
			container->end();

			check->value(m_devices[row].isOpen);

			check->onChange = [this, row](bool value)
			{
				if (value)
					c::config::openMidiDevice(m_type, row);
				else
					c::config::closeMidiDevice(m_type, row);
			};
		}
	}

	return container;
}

std::string geTabMidi::geDevices::setHeaderText(int col)
{
	if (col == 0)
		return g_ui->getI18Text(LangMap::CONFIG_MIDI_COLUMN_DEVICE);
	if (col == 1)
		return g_ui->getI18Text(LangMap::CONFIG_MIDI_COLUMN_ENABLE);
	return "";
}

void geTabMidi::geDevices::rebuild(const std::vector<m::KernelMidi::DeviceInfo>& devices)
{
	m_devices = devices;
	clear();
	prepareLayout();
	init();
}

void geTabMidi::geDevices::prepareLayout()
{
	rows(static_cast<int>(m_devices.size()));
	row_header(false);
	row_height_all(G_GUI_UNIT + PADDING * 2);
	row_resize(false);

	cols(2);
	col_header(true);
	col_resize(true);
	col_width(0, 350);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geTabMidi::geTabMidi(geompp::Rect<int> bounds)
: Fl_Group(bounds.x, bounds.y, bounds.w, bounds.h, g_ui->getI18Text(LangMap::CONFIG_MIDI_TITLE))
, m_data(c::config::getMidiData())
{
	end();

	const int LABEL_WIDTH = 120;

	geFlex* body = new geFlex(bounds.reduced(G_GUI_OUTER_MARGIN), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		geFlex* line0 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			m_system = new geChoice();
			line0->addWidget(new geBox(g_ui->getI18Text(LangMap::CONFIG_MIDI_SYSTEM), FL_ALIGN_RIGHT), LABEL_WIDTH);
			line0->addWidget(m_system);
			line0->end();
		}

		geFlex* line1 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			m_devicesOut = new geDevices(m_data.availableOutDevices, c::config::DeviceType::OUTPUT);
			line1->addWidget(new geBox(g_ui->getI18Text(LangMap::CONFIG_MIDI_OUTPUTDEVICES), FL_ALIGN_RIGHT), LABEL_WIDTH);
			line1->addWidget(m_devicesOut);
			line1->end();
		}

		geFlex* line2 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			m_devicesIn = new geDevices(m_data.availableInDevices, c::config::DeviceType::INPUT);
			line2->addWidget(new geBox(g_ui->getI18Text(LangMap::CONFIG_MIDI_INPUTDEVICES), FL_ALIGN_RIGHT), LABEL_WIDTH);
			line2->addWidget(m_devicesIn);
			line2->end();
		}

		geFlex* line3 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			m_midiMap = new geStringMenu(g_ui->getI18Text(LangMap::CONFIG_MIDI_NOMIDIMAPSFOUND));
			line3->addWidget(new geBox(g_ui->getI18Text(LangMap::CONFIG_MIDI_OUTPUTMIDIMAP), FL_ALIGN_RIGHT), LABEL_WIDTH);
			line3->addWidget(m_midiMap);
			line3->end();
		}

		geFlex* line4 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			m_sync = new geChoice();
			line4->addWidget(new geBox(g_ui->getI18Text(LangMap::CONFIG_MIDI_SYNC), FL_ALIGN_RIGHT), LABEL_WIDTH);
			line4->addWidget(m_sync);
			line4->end();
		}

		body->addWidget(line0, G_GUI_UNIT);
		body->addWidget(line1);
		body->addWidget(line2);
		body->addWidget(line3, G_GUI_UNIT);
		body->addWidget(line4, G_GUI_UNIT);
		body->end();
	}

	add(body);
	resizable(body);

	m_system->onChange = [this](ID id)
	{
		c::config::changeMidiAPI(static_cast<RtMidi::Api>(id));
		rebuild(c::config::getMidiData());
	};

	m_midiMap->onChange = [this](ID id)
	{
		m_data.selectedMidiMap = id;
		c::config::setMidiMapPath(m_data.getMidiMapByIndex(id));
	};

	m_sync->onChange = [this](ID id)
	{
		m_data.selectedSyncMode = id;
		c::config::setMidiSyncMode(m_data.selectedSyncMode);
	};

	rebuild(c::config::getMidiData());
}

/* -------------------------------------------------------------------------- */

void geTabMidi::rebuild(const c::config::MidiData& data)
{
	m_data = data;

	m_devicesOut->rebuild(m_data.availableOutDevices);
	m_devicesIn->rebuild(m_data.availableInDevices);

	m_system->clear();
	for (const auto& [key, value] : m_data.availableApis)
		m_system->addItem(value.c_str(), key);
	if (m_system->hasItem(m_data.selectedApi)) // Selected API might not be present in available APIs
		m_system->showItem(m_data.selectedApi);

	m_midiMap->rebuild(m_data.availableMidiMaps);
	const std::size_t midiMapIndex = u::vector::indexOf(m_data.availableMidiMaps, m_data.selectedMidiMap);
	if (midiMapIndex < m_data.availableMidiMaps.size())
		m_midiMap->showItem(static_cast<int>(midiMapIndex));

	m_sync->clear();
	for (const auto& [key, value] : m_data.availableSyncModes)
		m_sync->addItem(value.c_str(), key);
	m_sync->showItem(m_data.selectedSyncMode);
}
} // namespace giada::v
