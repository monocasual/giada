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

#include "tabAudio.h"
#include "core/const.h"
#include "core/kernelAudio.h"
#include "deps/rtaudio/RtAudio.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/input.h"
#include "utils/string.h"
#include <string>

namespace giada::v
{
geTabAudio::geDeviceMenu::geDeviceMenu(int x, int y, int w, int h, const char* l, const std::vector<c::config::AudioDeviceData>& devices)
: geChoice(x, y, w, h, l)
{
	if (devices.size() == 0)
	{
		addItem("-- no devices found --", 0);
		showItem(0);
		return;
	}

	for (const c::config::AudioDeviceData& device : devices)
		addItem(device.name, device.index);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geTabAudio::geChannelMenu::geChannelMenu(int x, int y, int w, int h, const char* l, c::config::AudioDeviceData& data)
: geChoice(x, y, w, h, l)
, m_data(data)
{
}

/* -------------------------------------------------------------------------- */

int geTabAudio::geChannelMenu::getChannelsCount() const
{
	return getSelectedId() < STEREO_OFFSET ? 1 : 2;
}

int geTabAudio::geChannelMenu::getChannelsStart() const
{
	if (m_data.channelsCount == 1)
		return getSelectedId();
	return getSelectedId() < STEREO_OFFSET ? getSelectedId() : getSelectedId() - STEREO_OFFSET;
}

/* -------------------------------------------------------------------------- */

void geTabAudio::geChannelMenu::rebuild(c::config::AudioDeviceData& data)
{
	m_data = data;

	clear();

	if (m_data.index == -1 /*|| m_data.channelsCount == 0*/)
	{
		addItem("none", 0);
		showItem(0);
		return;
	}

	if (m_data.type == c::config::DeviceType::INPUT)
		for (int i = 0; i < m_data.channelsMax; i++)
			addItem(std::to_string(i + 1), i);

	/* Dirty trick for stereo channels: they start at STEREO_OFFSET. */

	for (int i = 0; i < m_data.channelsMax; i += 2)
		addItem(std::to_string(i + 1) + "-" + std::to_string(i + 2), i + STEREO_OFFSET);

	if (m_data.channelsCount == 1)
		showItem(m_data.channelsStart);
	else
		showItem(m_data.channelsStart + STEREO_OFFSET);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geTabAudio::geTabAudio(int X, int Y, int W, int H)
: Fl_Group(X, Y, W, H, "Sound System")
, m_data(c::config::getAudioData())
, m_initialApi(m_data.api)
{
	begin();
	soundsys        = new geChoice(x() + 114, y() + 9, 250, 20, "System");
	buffersize      = new geChoice(x() + 114, y() + 37, 55, 20, "Buffer size");
	samplerate      = new geChoice(x() + 304, y() + 37, 60, 20, "Sample rate");
	sounddevOut     = new geDeviceMenu(x() + 114, y() + 65, 250, 20, "Output device", m_data.outputDevices);
	channelsOut     = new geChannelMenu(x() + 114, y() + 93, 55, 20, "Output channels", m_data.outputDevice);
	limitOutput     = new geCheck(x() + 177, y() + 93, 100, 20, "Limit output");
	sounddevIn      = new geDeviceMenu(x() + 114, y() + 121, 234, 20, "Input device", m_data.inputDevices);
	enableIn        = new geCheck(sounddevIn->x() + sounddevIn->w() + 4, sounddevIn->y(), 12, 20);
	channelsIn      = new geChannelMenu(x() + 114, y() + 149, 55, 20, "Input channels", m_data.inputDevice);
	recTriggerLevel = new geInput(x() + 309, y() + 149, 55, 20, "Rec threshold (dB)");
	rsmpQuality     = new geChoice(x() + 114, y() + 177, 250, 20, "Resampling");
	new geBox(x(), rsmpQuality->y() + rsmpQuality->h() + 8, w(), 92, "Restart Giada for the changes to take effect.");
	end();

	labelsize(G_GUI_FONT_SIZE_BASE);
	selection_color(G_COLOR_GREY_4);

	for (const auto& [key, value] : m_data.apis)
		soundsys->addItem(value.c_str(), key);
	soundsys->showItem(m_data.api);
	soundsys->onChange = [this](ID id) { m_data.api = id; invalidate(); };

	samplerate->onChange = [this](ID id) { m_data.sampleRate = id; };

	sounddevOut->showItem(m_data.outputDevice.index);
	sounddevOut->onChange = [this](ID id) { m_data.setOutputDevice(id); fetch(); };

	sounddevIn->showItem(m_data.inputDevice.index);
	sounddevIn->onChange = [this](ID id) { m_data.setInputDevice(id); fetch(); };

	enableIn->copy_tooltip("Enable Input");
	enableIn->value(m_data.inputDevice.index != -1);
	enableIn->onChange = [this](bool b) { m_data.setInputDevice(b ? 0 : -1); fetch(); };

	channelsOut->onChange = [this](ID) {
		m_data.outputDevice.channelsCount = channelsOut->getChannelsCount();
		m_data.outputDevice.channelsStart = channelsOut->getChannelsStart();
	};

	channelsIn->onChange = [this](ID) {
		m_data.inputDevice.channelsCount = channelsIn->getChannelsCount();
		m_data.inputDevice.channelsStart = channelsIn->getChannelsStart();
	};

	limitOutput->value(m_data.limitOutput);
	limitOutput->onChange = [this](bool v) { m_data.limitOutput = v; };

	buffersize->addItem("8", 8);
	buffersize->addItem("16", 16);
	buffersize->addItem("32", 32);
	buffersize->addItem("64", 64);
	buffersize->addItem("128", 128);
	buffersize->addItem("256", 256);
	buffersize->addItem("512", 512);
	buffersize->addItem("1024", 1024);
	buffersize->addItem("2048", 2048);
	buffersize->addItem("4096", 4096);
	buffersize->showItem(m_data.bufferSize);
	buffersize->onChange = [this](ID id) { m_data.bufferSize = id; };

	rsmpQuality->addItem("Sinc best quality (very slow)", 0);
	rsmpQuality->addItem("Sinc medium quality (slow)", 1);
	rsmpQuality->addItem("Sinc basic quality (medium)", 2);
	rsmpQuality->addItem("Zero Order Hold (fast)", 3);
	rsmpQuality->addItem("Linear (very fast)", 4);
	rsmpQuality->showItem(m_data.resampleQuality);
	rsmpQuality->onChange = [this](ID id) { m_data.resampleQuality = id; };

	recTriggerLevel->value(u::string::fToString(m_data.recTriggerLevel, 1).c_str());
	recTriggerLevel->onChange = [this](const std::string& s) { m_data.recTriggerLevel = std::stof(s); };

	if (m_data.api == G_SYS_API_NONE)
		deactivateAll();
	else
		fetch();
}

/* -------------------------------------------------------------------------- */

void geTabAudio::invalidate()
{
	/* If the user changes sound system (e.g. ALSA->JACK), deactivate all widgets. */

	if (m_initialApi == m_data.api && m_initialApi != -1 && m_data.api != G_SYS_API_NONE)
		activateAll();
	else
		deactivateAll();
}

/* -------------------------------------------------------------------------- */

void geTabAudio::fetch()
{
	for (int sampleRate : m_data.outputDevice.sampleRates)
		samplerate->addItem(std::to_string(sampleRate), sampleRate);
	samplerate->showItem(m_data.sampleRate);

	channelsOut->rebuild(m_data.outputDevice);
	m_data.outputDevice.channelsCount = channelsOut->getChannelsCount();
	m_data.outputDevice.channelsStart = channelsOut->getChannelsStart();

	if (m_data.inputDevice.index != -1)
	{
		channelsIn->rebuild(m_data.inputDevice);
		m_data.inputDevice.channelsCount = channelsIn->getChannelsCount();
		m_data.inputDevice.channelsStart = channelsIn->getChannelsStart();
		sounddevIn->activate();
		channelsIn->activate();
		recTriggerLevel->activate();
	}
	else
	{
		sounddevIn->deactivate();
		channelsIn->deactivate();
		recTriggerLevel->deactivate();
	}
}

/* -------------------------------------------------------------------------- */

void geTabAudio::deactivateAll()
{
	buffersize->deactivate();
	limitOutput->deactivate();
	sounddevOut->deactivate();
	channelsOut->deactivate();
	samplerate->deactivate();
	sounddevIn->deactivate();
	channelsIn->deactivate();
	recTriggerLevel->deactivate();
	rsmpQuality->deactivate();
}

/* -------------------------------------------------------------------------- */

void geTabAudio::activateAll()
{
	buffersize->activate();
	limitOutput->activate();
	sounddevOut->activate();
	channelsOut->activate();
	samplerate->activate();
	rsmpQuality->activate();
	if (m_data.inputDevice.index != -1)
	{
		sounddevIn->activate();
		channelsIn->activate();
		recTriggerLevel->activate();
	}
}

/* -------------------------------------------------------------------------- */

void geTabAudio::save()
{
	c::config::save(m_data);
}
} // namespace giada::v