/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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
#include "utils/gui.h"
#include <string>

namespace giada::v
{
geTabMidi::geMenu::geMenu(int x, int y, int w, int h, const char* l,
    const std::vector<std::string>& data, const std::string& msgIfNotFound)
: geChoice(x, y, w, h, l)
{
	if (data.size() == 0)
	{
		addItem(msgIfNotFound.c_str(), 0);
		showItem(0);
		deactivate();
	}
	else
	{
		for (const std::string& d : data)
			addItem(u::gui::removeFltkChars(d).c_str(), -1); // -1: auto-increment ID
	}
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geTabMidi::geTabMidi(int X, int Y, int W, int H)
: Fl_Group(X, Y, W, H, "MIDI")
, m_data(c::config::getMidiData())
, m_initialApi(m_data.api)
{
	begin();
	system    = new geChoice(x() + w() - 250, y() + 9, 250, 20, "System");
	portOut   = new geMenu(x() + w() - 250, system->y() + system->h() + 8, 234, 20, "Output port", m_data.outPorts, "-- no ports found --");
	enableOut = new geCheck(portOut->x() + portOut->w() + 4, portOut->y(), 12, 20);
	portIn    = new geMenu(x() + w() - 250, portOut->y() + portOut->h() + 8, 234, 20, "Input port", m_data.inPorts, "-- no ports found --");
	enableIn  = new geCheck(portIn->x() + portIn->w() + 4, portIn->y(), 12, 20);
	midiMap   = new geMenu(x() + w() - 250, portIn->y() + portIn->h() + 8, 250, 20, "Output Midi Map", m_data.midiMaps, "(no MIDI maps available)");
	sync      = new geChoice(x() + w() - 250, midiMap->y() + midiMap->h() + 8, 250, 20, "Sync");
	new geBox(x(), sync->y() + sync->h() + 8, w(), h() - 150, "Restart Giada for the changes to take effect.");
	end();

	labelsize(G_GUI_FONT_SIZE_BASE);
	selection_color(G_COLOR_GREY_4);

	for (const auto& [key, value] : m_data.apis)
		system->addItem(value.c_str(), key);
	system->showItem(m_data.api);
	system->onChange = [this](ID id) { m_data.api = id; invalidate(); };

	portOut->showItem(m_data.outPort);
	portOut->onChange = [this](ID id) { m_data.outPort = id; };
	if (m_data.outPort == -1)
		portOut->deactivate();

	portIn->showItem(m_data.inPort);
	portIn->onChange = [this](ID id) { m_data.inPort = id; };
	if (m_data.inPort == -1)
		portIn->deactivate();

	enableOut->copy_tooltip("Enable Output port");
	enableOut->value(m_data.outPort != -1);
	enableOut->onChange = [this](bool b) {
		if (b)
		{
			m_data.outPort = portOut->value();
			portOut->activate();
		}
		else
		{
			m_data.outPort = -1;
			portOut->deactivate();
		}
	};

	enableIn->copy_tooltip("Enable Input port");
	enableIn->value(m_data.inPort != -1);
	enableIn->onChange = [this](bool b) {
		if (b)
		{
			m_data.inPort = portIn->value();
			portIn->activate();
		}
		else
		{
			m_data.inPort = -1;
			portIn->deactivate();
		}
	};

	midiMap->showItem(m_data.midiMap);
	midiMap->onChange = [this](ID id) { m_data.midiMap = id; };

	for (const auto& [key, value] : m_data.syncModes)
		sync->addItem(value.c_str(), key);
	sync->showItem(m_data.syncMode);
	sync->onChange = [this](ID id) { m_data.syncMode = id; };
}

/* -------------------------------------------------------------------------- */

void geTabMidi::invalidate()
{
	/* If the user changes MIDI device (eg ALSA->JACK) device menu deactivates. 
	If it returns to the original system, we re-fill the list by re-using
	previous data. */

	if (m_initialApi == m_data.api && m_initialApi != -1)
	{
		portOut->activate();
		portIn->activate();
		enableOut->activate();
		enableIn->activate();
		if (m_data.midiMaps.size() > 0)
			midiMap->activate();
		sync->activate();
	}
	else
	{
		portOut->deactivate();
		portIn->deactivate();
		enableOut->deactivate();
		enableIn->deactivate();
		midiMap->deactivate();
		sync->deactivate();
	}
}

/* -------------------------------------------------------------------------- */

void geTabMidi::save() const
{
	c::config::save(m_data);
}
} // namespace giada::v
