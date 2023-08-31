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

#include "gui/elems/config/tabMidi.h"
#include "core/const.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/choice.h"
#include "gui/elems/basics/textButton.h"
#include "gui/elems/config/stringMenu.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include <string>

constexpr int LABEL_WIDTH = 120;

extern giada::v::Ui* g_ui;

namespace giada::v
{
geTabMidi::geTabMidi(geompp::Rect<int> bounds)
: Fl_Group(bounds.x, bounds.y, bounds.w, bounds.h, g_ui->getI18Text(LangMap::CONFIG_MIDI_TITLE))
, m_data(c::config::getMidiData())
{
	end();

	geFlex* body = new geFlex(bounds.reduced(G_GUI_OUTER_MARGIN), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		m_system = new geChoice(g_ui->getI18Text(LangMap::CONFIG_MIDI_SYSTEM), LABEL_WIDTH);

		geFlex* line1 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			m_portOut   = new geStringMenu(g_ui->getI18Text(LangMap::CONFIG_MIDI_OUTPUTPORT),
                g_ui->getI18Text(LangMap::CONFIG_MIDI_NOPORTSFOUND), LABEL_WIDTH);
			m_enableOut = new geCheck(0, 0, 0, 0);

			line1->addWidget(m_portOut);
			line1->addWidget(m_enableOut, 12);
			line1->end();
		}

		geFlex* line2 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			m_portIn   = new geStringMenu(g_ui->getI18Text(LangMap::CONFIG_MIDI_INPUTPORT),
                g_ui->getI18Text(LangMap::CONFIG_MIDI_NOPORTSFOUND), LABEL_WIDTH);
			m_enableIn = new geCheck(0, 0, 0, 0);

			line2->addWidget(m_portIn);
			line2->addWidget(m_enableIn, 12);
			line2->end();
		}

		geFlex* col1 = new geFlex(Direction::VERTICAL);
		{
			geFlex* line5 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
			{
				m_applyBtn = new geTextButton(g_ui->getI18Text(LangMap::COMMON_APPLY));

				line5->addWidget(new geBox());
				line5->addWidget(m_applyBtn, 80);
				line5->addWidget(new geBox());
				line5->end();
			}

			col1->addWidget(new geBox());
			col1->addWidget(line5, G_GUI_UNIT);
			col1->addWidget(new geBox());
			col1->end();
		}

		m_midiMap = new geStringMenu(g_ui->getI18Text(LangMap::CONFIG_MIDI_OUTPUTMIDIMAP),
		    g_ui->getI18Text(LangMap::CONFIG_MIDI_NOMIDIMAPSFOUND), LABEL_WIDTH);
		m_sync    = new geChoice(g_ui->getI18Text(LangMap::CONFIG_MIDI_SYNC), LABEL_WIDTH);

		body->addWidget(m_system, 20);
		body->addWidget(line1, 20);
		body->addWidget(line2, 20);
		body->addWidget(m_midiMap, 20);
		body->addWidget(m_sync, 20);
		body->addWidget(col1);
		body->end();
	}

	add(body);
	resizable(body);

	m_system->onChange = [this](ID id) {
		c::config::changeMidiAPI(static_cast<RtMidi::Api>(id));
		rebuild(c::config::getMidiData());
	};

	m_portOut->onChange = [this](ID id) { m_data.outPort = id; };

	m_portIn->onChange = [this](ID id) { m_data.inPort = id; };

	m_enableOut->copy_tooltip(g_ui->getI18Text(LangMap::CONFIG_MIDI_LABEL_ENABLEOUT));
	m_enableOut->onChange = [this](bool b) {
		if (m_data.outPorts.empty())
			return;
		if (b)
		{
			m_data.outPort = m_portOut->getSelectedId();
			m_portOut->activate();
		}
		else
		{
			m_data.outPort = -1;
			m_portOut->deactivate();
		}
	};

	m_enableIn->copy_tooltip(g_ui->getI18Text(LangMap::CONFIG_MIDI_LABEL_ENABLEIN));
	m_enableIn->onChange = [this](bool b) {
		if (m_data.inPorts.empty())
			return;
		if (b)
		{
			m_data.inPort = m_portIn->getSelectedId();
			m_portIn->activate();
		}
		else
		{
			m_data.inPort = -1;
			m_portIn->deactivate();
		}
	};

	m_midiMap->onChange = [this](ID id) {
		m_data.midiMap = id;
		c::config::setMidiMapPath(m_data.getMidiMapByIndex(id));
	};

	m_sync->onChange = [this](ID id) {
		m_data.syncMode = id;
		c::config::setMidiSyncMode(m_data.syncMode);
	};

	m_applyBtn->onClick = [this]() { c::config::apply(m_data); };

	rebuild(c::config::getMidiData());
}

/* -------------------------------------------------------------------------- */

void geTabMidi::rebuild(const c::config::MidiData& data)
{
	m_data = data;

	m_system->clear();
	for (const auto& [key, value] : m_data.apis)
		m_system->addItem(value.c_str(), key);
	m_system->showItem(m_data.api);

	m_portOut->rebuild(m_data.outPorts);
	if (m_data.outPort == -1)
	{
		m_portOut->showItem(0);
		m_portOut->deactivate();
	}
	else
		m_portOut->showItem(m_data.outPort);

	m_portIn->rebuild(m_data.inPorts);
	if (m_data.inPort == -1)
	{
		m_portIn->showItem(0);
		m_portIn->deactivate();
	}
	else
		m_portIn->showItem(m_data.inPort);

	m_enableOut->value(m_data.outPort != -1);
	if (m_data.outPorts.empty())
		m_enableOut->deactivate();
	else
		m_enableOut->activate();

	m_enableIn->value(m_data.inPort != -1);
	if (m_data.inPorts.empty())
		m_enableIn->deactivate();
	else
		m_enableIn->activate();

	m_midiMap->rebuild(m_data.midiMaps);
	const std::size_t midiMapIndex = u::vector::indexOf(m_data.midiMaps, m_data.midiMap);
	if (midiMapIndex < m_data.midiMaps.size())
		m_midiMap->showItem(midiMapIndex);

	m_sync->clear();
	for (const auto& [key, value] : m_data.syncModes)
		m_sync->addItem(value.c_str(), key);
	m_sync->showItem(m_data.syncMode);
}
} // namespace giada::v
