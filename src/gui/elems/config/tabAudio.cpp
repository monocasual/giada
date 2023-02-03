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

#include "tabAudio.h"
#include "core/const.h"
#include "core/kernelAudio.h"
#include "deps/rtaudio/RtAudio.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/choice.h"
#include "gui/elems/basics/flex.h"
#include "gui/elems/basics/input.h"
#include "gui/ui.h"
#include "utils/string.h"
#include <string>

constexpr int LABEL_WIDTH = 110;

extern giada::v::Ui g_ui;

namespace giada::v
{
geTabAudio::geDeviceMenu::geDeviceMenu(const char* l, const std::vector<c::config::AudioDeviceData>& devices)
: geChoice(l, LABEL_WIDTH)
{
	if (devices.size() == 0)
	{
		addItem(g_ui.getI18Text(LangMap::CONFIG_AUDIO_NODEVICESFOUND), 0);
		showItem(0);
		return;
	}

	for (const c::config::AudioDeviceData& device : devices)
		addItem(device.name, device.index);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geTabAudio::geChannelMenu::geChannelMenu(const char* l, const c::config::AudioDeviceData& data)
: geChoice(l, LABEL_WIDTH)
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

void geTabAudio::geChannelMenu::rebuild(const c::config::AudioDeviceData& data)
{
	m_data = data;

	clear();

	if (m_data.index == -1)
	{
		addItem(g_ui.getI18Text(LangMap::COMMON_NONE), 0);
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

geTabAudio::geTabAudio(geompp::Rect<int> bounds)
: Fl_Group(bounds.x, bounds.y, bounds.w, bounds.h, g_ui.getI18Text(LangMap::CONFIG_AUDIO_TITLE))
, m_data(c::config::getAudioData())
, m_initialApi(m_data.api)
{
	end();

	geFlex* body = new geFlex(bounds.reduced(G_GUI_OUTER_MARGIN), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		soundsys = new geChoice(g_ui.getI18Text(LangMap::CONFIG_AUDIO_SYSTEM), LABEL_WIDTH);

		geFlex* line1 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			buffersize = new geChoice(g_ui.getI18Text(LangMap::CONFIG_AUDIO_BUFFERSIZE), LABEL_WIDTH);
			samplerate = new geChoice(g_ui.getI18Text(LangMap::CONFIG_AUDIO_SAMPLERATE), LABEL_WIDTH);

			line1->add(buffersize, 180);
			line1->add(samplerate, 180);
			line1->end();
		}

		sounddevOut = new geDeviceMenu(g_ui.getI18Text(LangMap::CONFIG_AUDIO_OUTPUTDEVICE), m_data.outputDevices);

		geFlex* line2 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			channelsOut = new geChannelMenu(g_ui.getI18Text(LangMap::CONFIG_AUDIO_OUTPUTCHANNELS), m_data.outputDevice);
			limitOutput = new geCheck(x() + 177, y() + 93, 100, 20, g_ui.getI18Text(LangMap::CONFIG_AUDIO_LIMITOUTPUT));

			line2->add(channelsOut, 180);
			line2->add(limitOutput);
			line2->end();
		}

		geFlex* line3 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			sounddevIn = new geDeviceMenu(g_ui.getI18Text(LangMap::CONFIG_AUDIO_INPUTDEVICE), m_data.inputDevices);
			enableIn   = new geCheck(0, 0, 0, 0);

			line3->add(sounddevIn);
			line3->add(enableIn, 12);
			line3->end();
		}

		geFlex* line4 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			channelsIn      = new geChannelMenu(g_ui.getI18Text(LangMap::CONFIG_AUDIO_INPUTCHANNELS), m_data.inputDevice);
			recTriggerLevel = new geInput(g_ui.getI18Text(LangMap::CONFIG_AUDIO_RECTHRESHOLD), 120);

			line4->add(channelsIn, 180);
			line4->add(recTriggerLevel, 180);
			line4->end();
		}

		rsmpQuality = new geChoice(g_ui.getI18Text(LangMap::CONFIG_AUDIO_RESAMPLING), LABEL_WIDTH);

		body->add(soundsys, 20);
		body->add(line1, 20);
		body->add(sounddevOut, 20);
		body->add(line2, 20);
		body->add(line3, 20);
		body->add(line4, 20);
		body->add(rsmpQuality, 20);
		body->add(new geBox(g_ui.getI18Text(LangMap::CONFIG_RESTARTGIADA)));
		body->end();
	}

	add(body);
	resizable(body);

	for (const auto& [key, value] : m_data.apis)
		soundsys->addItem(value.c_str(), key);
	soundsys->showItem(m_data.api);
	soundsys->onChange = [this](ID id) {
		m_data.api = static_cast<RtAudio::Api>(id);
		invalidate();
	};

	samplerate->onChange = [this](ID id) { m_data.sampleRate = id; };

	sounddevOut->showItem(m_data.outputDevice.index);
	sounddevOut->onChange = [this](ID id) { m_data.setOutputDevice(id); fetch(); };

	sounddevIn->showItem(m_data.inputDevice.index);
	sounddevIn->onChange = [this](ID id) { m_data.setInputDevice(id); fetch(); };

	enableIn->copy_tooltip(g_ui.getI18Text(LangMap::CONFIG_AUDIO_ENABLEINPUT));
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

	rsmpQuality->addItem(g_ui.getI18Text(LangMap::CONFIG_AUDIO_RESAMPLING_SINCBEST), 0);
	rsmpQuality->addItem(g_ui.getI18Text(LangMap::CONFIG_AUDIO_RESAMPLING_SINCMEDIUM), 1);
	rsmpQuality->addItem(g_ui.getI18Text(LangMap::CONFIG_AUDIO_RESAMPLING_SINCBASIC), 2);
	rsmpQuality->addItem(g_ui.getI18Text(LangMap::CONFIG_AUDIO_RESAMPLING_ZEROORDER), 3);
	rsmpQuality->addItem(g_ui.getI18Text(LangMap::CONFIG_AUDIO_RESAMPLING_LINEAR), 4);
	rsmpQuality->showItem(m_data.resampleQuality);
	rsmpQuality->onChange = [this](ID id) { m_data.resampleQuality = id; };

	recTriggerLevel->setValue(u::string::fToString(m_data.recTriggerLevel, 1));
	recTriggerLevel->onChange = [this](const std::string& s) { m_data.recTriggerLevel = std::stof(s); };

	if (m_data.api == RtAudio::Api::RTAUDIO_DUMMY)
		deactivateAll();
	else
		fetch();
}

/* -------------------------------------------------------------------------- */

void geTabAudio::invalidate()
{
	/* If the user changes sound system (e.g. ALSA->JACK), deactivate all widgets. */

	if (m_initialApi == m_data.api && m_initialApi != -1 && m_data.api != RtAudio::Api::RTAUDIO_DUMMY)
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

	if (m_data.api == RtAudio::Api::UNIX_JACK)
		buffersize->deactivate();
	else
		buffersize->activate();

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