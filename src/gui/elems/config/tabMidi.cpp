/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#include <RtMidi.h>
#include <string>
#include "core/const.h"
#include "core/conf.h"
#include "core/midiMapConf.h"
#include "core/kernelMidi.h"
#include "core/midiPorts.h"
#include "utils/gui.h"
#include "utils/log.h"
#include "utils/vector.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/choice.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/menuButton.h"
#include "tabMidi.h"

namespace giada {
namespace v
{
geTabMidi::geTabMidi(int X, int Y, int W, int H)
: Fl_Group(X, Y, W, H, "MIDI")
{
	begin();
	system	= new geChoice(x()+w()-250, y()+9, 250, 20, "System");
	portOut	= new geMenuButton(x()+w()-250, system->y()+system->h()+8, 250, 20, "Output port", "Output ports");
	portIn	= new geMenuButton(x()+w()-250, portOut->y()+portOut->h()+8, 250, 20, "Input port", "Input Ports");
	midiMap	= new geChoice(x()+w()-250, portIn->y()+portIn->h()+8, 250, 20, "Output Midi Map");
	sync    = new geChoice(x()+w()-250, midiMap->y()+midiMap->h()+8, 250, 20, "Sync");
	new geBox(x(), sync->y()+sync->h()+8, w(), h()-150, "Restart Giada for the changes to take effect.");
	end();

	labelsize(G_GUI_FONT_SIZE_BASE);
	selection_color(G_COLOR_GREY_4);

	system->callback(cb_changeSystem, (void*)this);

	fetchSystems();
	populateOutPorts();
	populateInPorts();
	fetchMidiMaps();

	sync->add("(disabled)");
	sync->add("MIDI Clock (master)");
	sync->add("MTC (master)");
	if      (m::conf::conf.midiSync == MIDI_SYNC_NONE)
		sync->value(0);
	else if (m::conf::conf.midiSync == MIDI_SYNC_CLOCK_M)
		sync->value(1);
	else if (m::conf::conf.midiSync == MIDI_SYNC_MTC_M)
		sync->value(2);

	systemInitValue = system->value();
}


/* -------------------------------------------------------------------------- */

void geTabMidi::populateOutPorts()
{
	std::vector<std::string> ports = m::midiPorts::getOutDevices();
	int i;

	for (auto& p : ports)
		portOut->add(u::gui::removeFltkChars(p).c_str(), 0,
					cb_portOutMenu, 0, FL_MENU_TOGGLE);

	// Let's also add ports that are currently in config file but are
	// not present in the system
	
	for (std::string& p : m::conf::conf.midiPortOutNames) {
		if (!u::vector::has(ports, p)) {
			i = portOut->add(u::gui::removeFltkChars(p).c_str(),
			0, cb_portOutMenu, 0, FL_MENU_TOGGLE | FL_MENU_VALUE);
			//portOut->menu()[i].labelcolor(G_COLOR_GREY_4);
		}
	}
}

/* -------------------------------------------------------------------------- */


void geTabMidi::cb_portOutMenu(Fl_Widget *w, void*)
{
	// Does nothing, but could do something!
	return;
}

/* -------------------------------------------------------------------------- */

void geTabMidi::populateInPorts()
{
	std::vector<std::string> ports = m::midiPorts::getInDevices();

	for (auto& p : ports)
		portIn->add(u::gui::removeFltkChars(p).c_str(), 0,
					cb_portInMenu, 0, FL_MENU_TOGGLE);

}

/* -------------------------------------------------------------------------- */


void geTabMidi::cb_portInMenu(Fl_Widget *w, void*)
{
	// Does nothing, but could do something!
	return;
}

/* -------------------------------------------------------------------------- */


void geTabMidi::fetchMidiMaps()
{
	if (m::midimap::maps.empty()) {
		midiMap->add("(no MIDI maps available)");
		midiMap->value(0);
		midiMap->deactivate();
		return;
	}

	for (unsigned i=0; i<m::midimap::maps.size(); i++) {
		const char *imap = m::midimap::maps.at(i).c_str();
		midiMap->add(imap);
		if (m::conf::conf.midiMapPath == imap)
			midiMap->value(i);
	}

	/* Preselect the 0 m::midimap if nothing is selected but midimaps exist. */

	if (midiMap->value() == -1 && m::midimap::maps.size() > 0)
		midiMap->value(0);
}


/* -------------------------------------------------------------------------- */


void geTabMidi::save()
{
	std::string text = system->text(system->value());

	if      (text == "ALSA")
		m::conf::conf.midiSystem = RtMidi::LINUX_ALSA;
	else if (text == "Jack")
		m::conf::conf.midiSystem = RtMidi::UNIX_JACK;
	else if (text == "Multimedia MIDI")
		m::conf::conf.midiSystem = RtMidi::WINDOWS_MM;
	else if (text == "OSX Core MIDI")
		m::conf::conf.midiSystem = RtMidi::MACOSX_CORE;

	m::conf::conf.midiPortOutNames.clear();
u::log::print("%d <- portOut->size()\n", portOut->size());
	for (int i = 0; i < portOut->size(); i++) {
u::log::print("%d <- value pozycji %d\n", portOut->menu()[i].value(), i);
		if (portOut->menu()[i].value()) {
			m::conf::conf.midiPortOutNames.push_back(
					portOut->menu()[i].label());
		}
	}


	m::conf::conf.midiPortInNames.clear();
	for (int i = 0; i < portIn->size(); i++) {
		if (portIn->menu()[i].value()) {
			m::conf::conf.midiPortInNames.push_back(
					portIn->menu()[i].label());
		}
	}

	m::conf::conf.midiMapPath     = m::midimap::maps.size() == 0 ? "" : midiMap->text(midiMap->value());

	if      (sync->value() == 0)
		m::conf::conf.midiSync = MIDI_SYNC_NONE;
	else if (sync->value() == 1)
		m::conf::conf.midiSync = MIDI_SYNC_CLOCK_M;
	else if (sync->value() == 2)
		m::conf::conf.midiSync = MIDI_SYNC_MTC_M;
}


/* -------------------------------------------------------------------------- */


void geTabMidi::fetchSystems()
{
#if defined(__linux__)

	if (m::midiPorts::hasAPI(RtMidi::LINUX_ALSA))
		system->add("ALSA");
	if (m::midiPorts::hasAPI(RtMidi::UNIX_JACK))
		system->add("Jack");

#elif defined(__FreeBSD__)

	if (m::midiPorts::hasAPI(RtMidi::UNIX_JACK))
		system->add("Jack");

#elif defined(_WIN32)

	if (m::midiPorts::hasAPI(RtMidi::WINDOWS_MM))
		system->add("Multimedia MIDI");

#elif defined (__APPLE__)

	system->add("OSX Core MIDI");

#endif

	switch (m::conf::conf.midiSystem) {
		case RtMidi::LINUX_ALSA:  system->showItem("ALSA"); break;
		case RtMidi::UNIX_JACK:   system->showItem("Jack"); break;
		case RtMidi::WINDOWS_MM:  system->showItem("Multimedia MIDI"); break;
		case RtMidi::MACOSX_CORE: system->showItem("OSX Core MIDI"); break;
		default: system->value(0); break;
	}
}


/* -------------------------------------------------------------------------- */


void geTabMidi::cb_changeSystem(Fl_Widget* /*w*/, void* p) { ((geTabMidi*)p)->cb_changeSystem(); }


/* -------------------------------------------------------------------------- */


void geTabMidi::cb_changeSystem()
{
	/* if the user changes MIDI device (eg ALSA->JACK) device menu deactivates.
	 * If it returns to the original system, we re-fill the list by
	 * querying m::kernelMidi. */

	if (systemInitValue == system->value()) {
	//	portOut->clear();
	//	fetchOutPorts();
	//	TODO
	//	portOut->activate();
	//	portIn->clear();
	//	fetchInPorts();
	//	portIn->activate();
		sync->activate();
	}
	else {
	//	portOut->deactivate();
	//	portOut->clear();
	//	portOut->add("-- restart to fetch device(s) --");
	//	portOut->value(0);
	//	portIn->deactivate();
	//	portIn->clear();
	//	portIn->add("-- restart to fetch device(s) --");
	//	portIn->value(0);
		sync->deactivate();
	}
}
}} // giada::v::
