/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#include <string>
#include <rtmidi/RtMidi.h>
#include "../../../core/const.h"
#include "../../../core/conf.h"
#include "../../../core/midiMapConf.h"
#include "../../../core/kernelMidi.h"
#include "../../../utils/gui.h"
#include "../basics/box.h"
#include "../basics/choice.h"
#include "../basics/check.h"
#include "tabMidi.h"


using std::string;
using namespace giada::m;


geTabMidi::geTabMidi(int X, int Y, int W, int H)
	: Fl_Group(X, Y, W, H, "MIDI")
{
	begin();
	system	  = new geChoice(x()+92, y()+9, 253, 20, "System");
	portOut	  = new geChoice(x()+92, system->y()+system->h()+8, 253, 20, "Output port");
	portIn	  = new geChoice(x()+92, portOut->y()+portOut->h()+8, 253, 20, "Input port");
	noNoteOff = new geCheck (x()+92, portIn->y()+portIn->h()+8, 253, 20, "Device does not send NoteOff");
	midiMap	  = new geChoice(x()+92, noNoteOff->y()+noNoteOff->h(), 253, 20, "Output Midi Map");
	sync	    = new geChoice(x()+92, midiMap->y()+midiMap->h()+8, 253, 20, "Sync");
	new geBox(x(), sync->y()+sync->h()+8, w(), h()-125, "Restart Giada for the changes to take effect.");
	end();

	labelsize(GUI_FONT_SIZE_BASE);

	system->callback(cb_changeSystem, (void*)this);

	fetchSystems();
	fetchOutPorts();
	fetchInPorts();
	fetchMidiMaps();

	noNoteOff->value(conf::noNoteOff);

	sync->add("(disabled)");
	sync->add("MIDI Clock (master)");
	sync->add("MTC (master)");
	if      (conf::midiSync == MIDI_SYNC_NONE)
		sync->value(0);
	else if (conf::midiSync == MIDI_SYNC_CLOCK_M)
		sync->value(1);
	else if (conf::midiSync == MIDI_SYNC_MTC_M)
		sync->value(2);

	systemInitValue = system->value();
}


/* -------------------------------------------------------------------------- */


void geTabMidi::fetchOutPorts()
{
	if (kernelMidi::countOutPorts() == 0) {
		portOut->add("-- no ports found --");
		portOut->value(0);
		portOut->deactivate();
	}
	else {

		portOut->add("(disabled)");

		for (unsigned i=0; i<kernelMidi::countOutPorts(); i++)
			portOut->add(gu_removeFltkChars(kernelMidi::getOutPortName(i)).c_str());

		portOut->value(conf::midiPortOut+1);    // +1 because midiPortOut=-1 is '(disabled)'
	}
}

/* -------------------------------------------------------------------------- */


void geTabMidi::fetchInPorts()
{
	if (kernelMidi::countInPorts() == 0) {
		portIn->add("-- no ports found --");
		portIn->value(0);
		portIn->deactivate();
	}
	else {

		portIn->add("(disabled)");

		for (unsigned i=0; i<kernelMidi::countInPorts(); i++)
			portIn->add(gu_removeFltkChars(kernelMidi::getInPortName(i)).c_str());

		portIn->value(conf::midiPortIn+1);    // +1 because midiPortIn=-1 is '(disabled)'
	}
}


/* -------------------------------------------------------------------------- */


void geTabMidi::fetchMidiMaps()
{
	if (midimap::maps.size() == 0) {
		midiMap->add("(no MIDI maps available)");
		midiMap->value(0);
		midiMap->deactivate();
		return;
	}

	for (unsigned i=0; i<midimap::maps.size(); i++) {
		const char *imap = midimap::maps.at(i).c_str();
		midiMap->add(imap);
		if (conf::midiMapPath == imap)
			midiMap->value(i);
	}

	/* Preselect the 0 midimap if nothing is selected but midimaps exist. */

	if (midiMap->value() == -1 && midimap::maps.size() > 0)
		midiMap->value(0);
}


/* -------------------------------------------------------------------------- */


void geTabMidi::save()
{
	string text = system->text(system->value());

	if      (text == "ALSA")
		conf::midiSystem = RtMidi::LINUX_ALSA;
	else if (text == "Jack")
		conf::midiSystem = RtMidi::UNIX_JACK;
	else if (text == "Multimedia MIDI")
		conf::midiSystem = RtMidi::WINDOWS_MM;
	else if (text == "OSX Core MIDI")
		conf::midiSystem = RtMidi::MACOSX_CORE;

	conf::midiPortOut = portOut->value()-1;   // -1 because midiPortOut=-1 is '(disabled)'
	conf::midiPortIn  = portIn->value()-1;    // -1 because midiPortIn=-1 is '(disabled)'

	conf::noNoteOff   = noNoteOff->value();
	conf::midiMapPath = midimap::maps.size() == 0 ? "" : midiMap->text(midiMap->value());

	if      (sync->value() == 0)
		conf::midiSync = MIDI_SYNC_NONE;
	else if (sync->value() == 1)
		conf::midiSync = MIDI_SYNC_CLOCK_M;
	else if (sync->value() == 2)
		conf::midiSync = MIDI_SYNC_MTC_M;
}


/* -------------------------------------------------------------------------- */


void geTabMidi::fetchSystems()
{
#if defined(__linux__)

	if (kernelMidi::hasAPI(RtMidi::LINUX_ALSA))
		system->add("ALSA");
	if (kernelMidi::hasAPI(RtMidi::UNIX_JACK))
		system->add("Jack");

#elif defined(_WIN32)

	if (kernelMidi::hasAPI(RtMidi::WINDOWS_MM))
		system->add("Multimedia MIDI");

#elif defined (__APPLE__)

	system->add("OSX Core MIDI");

#endif

	switch (conf::midiSystem) {
		case RtMidi::LINUX_ALSA:  system->showItem("ALSA"); break;
		case RtMidi::UNIX_JACK:   system->showItem("Jack"); break;
		case RtMidi::WINDOWS_MM:  system->showItem("Multimedia MIDI"); break;
		case RtMidi::MACOSX_CORE: system->showItem("OSX Core MIDI"); break;
		default: system->value(0); break;
	}
}


/* -------------------------------------------------------------------------- */


void geTabMidi::cb_changeSystem(Fl_Widget *w, void *p) { ((geTabMidi*)p)->__cb_changeSystem(); }


/* -------------------------------------------------------------------------- */


void geTabMidi::__cb_changeSystem()
{
	/* if the user changes MIDI device (eg ALSA->JACK) device menu deactivates.
	 * If it returns to the original system, we re-fill the list by
	 * querying kernelMidi. */

	if (systemInitValue == system->value()) {
		portOut->clear();
		fetchOutPorts();
		portOut->activate();
		portIn->clear();
		fetchInPorts();
		portIn->activate();
		noNoteOff->activate();
		sync->activate();
	}
	else {
		portOut->deactivate();
		portOut->clear();
		portOut->add("-- restart to fetch device(s) --");
		portOut->value(0);
		portIn->deactivate();
		portIn->clear();
		portIn->add("-- restart to fetch device(s) --");
		portIn->value(0);
		noNoteOff->deactivate();
		sync->deactivate();
	}

}
