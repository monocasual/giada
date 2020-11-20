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


#include <string>
#include "deps/rtaudio/RtAudio.h"
#include "core/const.h"
#include "core/conf.h"
#include "core/kernelAudio.h"
#include "utils/string.h"
#include "gui/dialogs/devInfo.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/choice.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/basics/button.h"
#include "tabAudio.h"


namespace giada {
namespace v
{
geTabAudio::geTabAudio(int X, int Y, int W, int H)
: Fl_Group(X, Y, W, H, "Sound System")
{
	begin();
	soundsys        = new geChoice(x()+114, y()+9,  250, 20, "System");
	buffersize      = new geChoice(x()+114, y()+37, 55,  20, "Buffer size");
	samplerate      = new geChoice(x()+304, y()+37, 60,  20, "Sample rate");
	sounddevOut     = new geChoice(x()+114, y()+65, 222, 20, "Output device");
	devOutInfo      = new geButton(x()+344, y()+65, 20,  20, "?");
	channelsOut     = new geChoice(x()+114, y()+93, 55,  20, "Output channels");
	limitOutput     = new geCheck (x()+177, y()+93, 55,  20, "Limit output");
	sounddevIn      = new geChoice(x()+114, y()+121, 222, 20, "Input device");
	devInInfo       = new geButton(x()+344, y()+121, 20,  20, "?");
	channelsIn      = new geChoice(x()+114, y()+149, 55,  20, "Input channels");
	recTriggerLevel = new geInput (x()+309, y()+149, 55,  20, "Rec threshold (dB)");
	rsmpQuality     = new geChoice(x()+114, y()+177, 250, 20, "Resampling");
                      new geBox(x(), rsmpQuality->y()+rsmpQuality->h()+8, w(), 92, "Restart Giada for the changes to take effect.");
	end();

	labelsize(G_GUI_FONT_SIZE_BASE);
	selection_color(G_COLOR_GREY_4);

	soundsys->add("(none)");

#if defined(__linux__)

	if (m::kernelAudio::hasAPI(RtAudio::LINUX_ALSA))
		soundsys->add("ALSA");
	if (m::kernelAudio::hasAPI(RtAudio::UNIX_JACK))
		soundsys->add("Jack");
	if (m::kernelAudio::hasAPI(RtAudio::LINUX_PULSE))
		soundsys->add("PulseAudio");

	switch (m::conf::conf.soundSystem) {
		case G_SYS_API_NONE:
			soundsys->showItem("(none)");
			break;
		case G_SYS_API_ALSA:
			soundsys->showItem("ALSA");
			break;
		case G_SYS_API_JACK:
			soundsys->showItem("Jack");
			buffersize->deactivate();
			samplerate->deactivate();
			break;
		case G_SYS_API_PULSE:
			soundsys->showItem("PulseAudio");
			break;
	}

#elif defined(__FreeBSD__)

	if (m::kernelAudio::hasAPI(RtAudio::UNIX_JACK))
		soundsys->add("Jack");
	if (m::kernelAudio::hasAPI(RtAudio::LINUX_PULSE))
		soundsys->add("PulseAudio");

	switch (m::conf::conf.soundSystem) {
		case G_SYS_API_NONE:
			soundsys->showItem("(none)");
			break;
		case G_SYS_API_JACK:
			soundsys->showItem("Jack");
			buffersize->deactivate();
			samplerate->deactivate();
			break;
		case G_SYS_API_PULSE:
			soundsys->showItem("PulseAudio");
			break;
	}

#elif defined(_WIN32)

	if (m::kernelAudio::hasAPI(RtAudio::WINDOWS_DS))
		soundsys->add("DirectSound");
	if (m::kernelAudio::hasAPI(RtAudio::WINDOWS_ASIO))
		soundsys->add("ASIO");
	if (m::kernelAudio::hasAPI(RtAudio::WINDOWS_WASAPI))
		soundsys->add("WASAPI");

	switch (m::conf::conf.soundSystem) {
		case G_SYS_API_NONE:
			soundsys->showItem("(none)");
			break;
		case G_SYS_API_DS:
			soundsys->showItem("DirectSound");
			break;
		case G_SYS_API_ASIO:
			soundsys->showItem("ASIO");
			break;
		case G_SYS_API_WASAPI:
			soundsys->showItem("WASAPI");
			break;
	}

#elif defined(__APPLE__)

	if (m::kernelAudio::hasAPI(RtAudio::MACOSX_CORE))
		soundsys->add("CoreAudio");

	switch (m::conf::conf.soundSystem) {
		case G_SYS_API_NONE:
			soundsys->showItem("(none)");
			break;
		case G_SYS_API_CORE:
			soundsys->showItem("CoreAudio");
			break;
	}

#endif

	soundsysInitValue = soundsys->value();

	soundsys->callback(cb_deactivate_sounddev, (void*)this);

	sounddevIn->callback(cb_fetchInChans, this);
	sounddevOut->callback(cb_fetchOutChans, this);

	devOutInfo->callback(cb_showOutputInfo, this);
	devInInfo->callback(cb_showInputInfo, this);

	if (m::conf::conf.soundSystem != G_SYS_API_NONE) {
		fetchSoundDevs();
		fetchOutChans();
		fetchInChans(sounddevIn->value());

		/* fill frequency dropdown menu */
		/* TODO - add fetchFrequencies() */

		int nfreq = m::kernelAudio::getTotalFreqs(sounddevOut->value());
		for (int i=0; i<nfreq; i++) {
			int freq = m::kernelAudio::getFreq(sounddevOut->value(), i);
			samplerate->add(u::string::iToString(freq).c_str());
			if (freq == m::conf::conf.samplerate)
				samplerate->value(i);
		}
	}
	else {
		sounddevIn->deactivate();
		sounddevOut->deactivate();
		channelsIn->deactivate();
		channelsOut->deactivate();
		devOutInfo->deactivate();
		devInInfo->deactivate();
		samplerate->deactivate();
	}

	buffersize->add("8");
	buffersize->add("16");
	buffersize->add("32");
	buffersize->add("64");
	buffersize->add("128");
	buffersize->add("256");
	buffersize->add("512");
	buffersize->add("1024");
	buffersize->add("2048");
	buffersize->add("4096");
	buffersize->showItem(std::to_string(m::conf::conf.buffersize).c_str());

	rsmpQuality->add("Sinc best quality (very slow)");
	rsmpQuality->add("Sinc medium quality (slow)");
	rsmpQuality->add("Sinc basic quality (medium)");
	rsmpQuality->add("Zero Order Hold (fast)");
	rsmpQuality->add("Linear (very fast)");
	rsmpQuality->value(m::conf::conf.rsmpQuality);

	recTriggerLevel->value(u::string::fToString(m::conf::conf.recTriggerLevel, 1).c_str());

	limitOutput->value(m::conf::conf.limitOutput);
}


/* -------------------------------------------------------------------------- */


void geTabAudio::cb_deactivate_sounddev(Fl_Widget* /*w*/, void* p) { ((geTabAudio*)p)->cb_deactivate_sounddev(); }
void geTabAudio::cb_fetchInChans(Fl_Widget* /*w*/, void* p)        { ((geTabAudio*)p)->cb_fetchInChans(); }
void geTabAudio::cb_fetchOutChans(Fl_Widget* /*w*/, void* p)       { ((geTabAudio*)p)->cb_fetchOutChans(); }
void geTabAudio::cb_showInputInfo(Fl_Widget* /*w*/, void* p)       { ((geTabAudio*)p)->cb_showInputInfo(); }
void geTabAudio::cb_showOutputInfo(Fl_Widget* /*w*/, void* p)      { ((geTabAudio*)p)->cb_showOutputInfo(); }


/* -------------------------------------------------------------------------- */


void geTabAudio::cb_fetchInChans()
{
	fetchInChans(sounddevIn->value());
	channelsIn->value(0);
}


/* -------------------------------------------------------------------------- */


void geTabAudio::cb_fetchOutChans()
{
	fetchOutChans();
	channelsOut->value(0);
}


/* -------------------------------------------------------------------------- */


void geTabAudio::cb_showInputInfo()
{
	unsigned dev = m::kernelAudio::getDeviceByName(sounddevIn->text(sounddevIn->value()));
	new v::gdDevInfo(dev);
}


/* -------------------------------------------------------------------------- */


void geTabAudio::cb_showOutputInfo()
{
	unsigned dev = m::kernelAudio::getDeviceByName(sounddevOut->text(sounddevOut->value()));
	new v::gdDevInfo(dev);
}


/* -------------------------------------------------------------------------- */


void geTabAudio::cb_deactivate_sounddev()
{
	/* if the user changes sound system (eg ALSA->JACK) device menu deactivates.
	 * If it returns to the original sound system, we re-fill the list by
	 * querying m::kernelAudio. Watch out if soundsysInitValue == 0: you don't want
	 * to query m::kernelAudio for '(none)' soundsystem! */

	if (soundsysInitValue == soundsys->value() && soundsysInitValue != 0) {
		sounddevOut->clear();
		sounddevIn->clear();

		fetchSoundDevs();

		/* the '?' button is added by fetchSoundDevs */

		fetchOutChans();
		sounddevOut->activate();
		channelsOut->activate();

		/* chan menus and '?' button are activated by fetchInChans(...) */

		fetchInChans(sounddevIn->value());
		sounddevIn->activate();
		samplerate->activate();
	}
	else {
		sounddevOut->deactivate();
		sounddevOut->clear();
		sounddevOut->add("-- restart to fetch device(s) --");
		sounddevOut->value(0);
		channelsOut->deactivate();
		devOutInfo->deactivate();
		samplerate->deactivate();

		sounddevIn->deactivate();
		sounddevIn->clear();
		sounddevIn->add("-- restart to fetch device(s) --");
		sounddevIn->value(0);
		channelsIn->deactivate();
		devInInfo->deactivate();
	}
}

/* -------------------------------------------------------------------------- */


void geTabAudio::fetchInChans(int menuItem)
{
	/* if menuItem==0 device in input is disabled. */

	if (menuItem == 0) {
		devInInfo->deactivate();
		channelsIn->deactivate();
		recTriggerLevel->deactivate();
		return;
	}

	devInInfo->activate();
	channelsIn->activate();
	recTriggerLevel->activate();

	channelsIn->clear();

	unsigned dev = m::kernelAudio::getDeviceByName(sounddevIn->text(sounddevIn->value()));
	unsigned chs = m::kernelAudio::getMaxInChans(dev);

	if (chs == 0) {
		channelsIn->add("none");
		channelsIn->value(0);
		return;
	}

	/* Dirty trick for stereo inputs: indexes start from 1000. */

	for (unsigned i = 0; i < chs; i++) 
		channelsIn->addItem(std::to_string(i + 1).c_str(), i + 1);
	for (unsigned i = 0; i < chs; i += 2)
		channelsIn->addItem((std::to_string(i + 1) + "-" + std::to_string(i + 2)).c_str(), i + 1001);

	if (m::conf::conf.channelsInCount == 1)
		channelsIn->showItem(m::conf::conf.channelsInStart + 1);
	else
		channelsIn->showItem(m::conf::conf.channelsInStart + 1001);
}


/* -------------------------------------------------------------------------- */


void geTabAudio::fetchOutChans()
{
	channelsOut->clear();

	unsigned dev = m::kernelAudio::getDeviceByName(sounddevOut->text(sounddevOut->value()));
	unsigned chs = m::kernelAudio::getMaxOutChans(dev);

	if (chs == 0) {
		channelsOut->add("none");
		channelsOut->value(0);
		return;
	}
	for (unsigned i=0; i<chs; i+=2) {
		std::string tmp = u::string::iToString(i+1) + "-" + u::string::iToString(i+2);
		channelsOut->add(tmp.c_str());
	}
	channelsOut->value(m::conf::conf.channelsOut);
}


/* -------------------------------------------------------------------------- */


int geTabAudio::findMenuDevice(geChoice *m, int device)
{
	if (device == -1 || !m::kernelAudio::isReady())
		return 0;

	for (int i=0; i<m->size(); i++) {
		if (m::kernelAudio::getDeviceName(device) == "")
			continue;
		if (m->text(i) == nullptr)
			continue;
		if (m->text(i) == m::kernelAudio::getDeviceName(device))
			return i;
	}

	return 0;
}


/* -------------------------------------------------------------------------- */


void geTabAudio::fetchSoundDevs()
{
	if (m::kernelAudio::countDevices() == 0) {
		sounddevOut->add("-- no devices found --");
		sounddevOut->value(0);
		sounddevIn->add("-- no devices found --");
		sounddevIn->value(0);
		devInInfo ->deactivate();
		devOutInfo->deactivate();
	}
	else {

		devInInfo ->activate();
		devOutInfo->activate();

		/* input device may be disabled: now device number -1 is the disabled
		 * one. KernelAudio knows how to handle it. */

		sounddevIn->add("(disabled)");

		for (unsigned i=0; i<m::kernelAudio::countDevices(); i++) {

			/* escaping '/', very dangerous in FLTK (it creates a submenu) */

			std::string tmp = m::kernelAudio::getDeviceName(i);
			for (unsigned k=0; k<tmp.size(); k++)
				if (tmp[k] == '/' || tmp[k] == '|' || tmp[k] == '&' || tmp[k] == '_')
					tmp[k] = '-';

			/* add to list devices with at least 1 channel available. In this
			 * way we can filter devices only for input or output, e.g. an input
			 * devices has 0 output channels. */

			if (m::kernelAudio::getMaxOutChans(i) > 0)
				sounddevOut->add(tmp.c_str());

			if (m::kernelAudio::getMaxInChans(i) > 0)
				sounddevIn->add(tmp.c_str());
		}

		/* we show the device saved in the configuration file. */

		if (sounddevOut->size() == 0) {
			sounddevOut->add("-- no devices found --");
			sounddevOut->value(0);
			devOutInfo->deactivate();
		}
		else {
			int outMenuValue = findMenuDevice(sounddevOut, m::conf::conf.soundDeviceOut);
			sounddevOut->value(outMenuValue);
		}

		if (sounddevIn->size() == 0) {
			sounddevIn->add("-- no devices found --");
			sounddevIn->value(0);
			devInInfo->deactivate();
		}
		else {
			int inMenuValue = findMenuDevice(sounddevIn, m::conf::conf.soundDeviceIn);
			sounddevIn->value(inMenuValue);
		}
	}
}


/* -------------------------------------------------------------------------- */


void geTabAudio::save()
{
	std::string text = soundsys->text(soundsys->value());

	if (text == "(none)") {
		m::conf::conf.soundSystem = G_SYS_API_NONE;
		return;
	}

#if defined(__linux__)

	else if (text == "ALSA")
		m::conf::conf.soundSystem = G_SYS_API_ALSA;
	else if (text == "Jack")
		m::conf::conf.soundSystem = G_SYS_API_JACK;
	else if (text == "PulseAudio")
		m::conf::conf.soundSystem = G_SYS_API_PULSE;

#elif defined(__FreeBSD__)

	else if (text == "Jack")
		m::conf::conf.soundSystem = G_SYS_API_JACK;
	else if (text == "PulseAudio")
		m::conf::conf.soundSystem = G_SYS_API_PULSE;

#elif defined(_WIN32)

	else if (text == "DirectSound")
		m::conf::conf.soundSystem = G_SYS_API_DS;
	else if (text == "ASIO")
		m::conf::conf.soundSystem = G_SYS_API_ASIO;
	else if (text == "WASAPI")
		m::conf::conf.soundSystem = G_SYS_API_WASAPI;

#elif defined(__APPLE__)

	else if (text == "CoreAudio")
		m::conf::conf.soundSystem = G_SYS_API_CORE;

#endif

	/* use the device name to search into the drop down menu's */

	m::conf::conf.soundDeviceOut  = m::kernelAudio::getDeviceByName(sounddevOut->text(sounddevOut->value()));
	m::conf::conf.soundDeviceIn   = m::kernelAudio::getDeviceByName(sounddevIn->text(sounddevIn->value()));
	m::conf::conf.channelsOut     = channelsOut->value();
	m::conf::conf.channelsInCount = channelsIn->getSelectedId() < 1000 ? 1 : 2;
	m::conf::conf.channelsInStart = channelsIn->getSelectedId() - (m::conf::conf.channelsInCount == 1 ? 1 : 1001);
	m::conf::conf.limitOutput     = limitOutput->value();
	m::conf::conf.rsmpQuality     = rsmpQuality->value();

	/* If sounddevOut is disabled because of system change e.g. alsa -> jack, 
	soundDeviceOut and channelsOut are == -1. Change them! */

	if (m::conf::conf.soundDeviceOut == -1) {
		m::conf::conf.soundDeviceOut = 0;
		m::conf::conf.channelsOut    = 0;
	}

	m::conf::conf.buffersize      = std::atoi(buffersize->text());
	m::conf::conf.recTriggerLevel = std::atof(recTriggerLevel->value());

	const Fl_Menu_Item* i = nullptr;
	i = samplerate->mvalue(); // mvalue() returns a pointer to the last menu item that was picked
	if (i != nullptr)
		m::conf::conf.samplerate = std::atoi(i->label());
}
}} // giada::v::
