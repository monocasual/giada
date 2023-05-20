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
#include "gui/elems/basics/textButton.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include <fmt/core.h>
#include <string>

constexpr int LABEL_WIDTH = 110;

extern giada::v::Ui g_ui;

namespace giada::v
{
geTabAudio::geDeviceMenu::geDeviceMenu(const char* l)
: geChoice(l, LABEL_WIDTH)
{
}

/* -------------------------------------------------------------------------- */

void geTabAudio::geDeviceMenu::rebuild(const std::vector<c::config::AudioDeviceData>& devices)
{
	clear();

	if (devices.size() == 0)
	{
		addItem(g_ui.getI18Text(LangMap::CONFIG_AUDIO_NODEVICESFOUND), 0);
		showItem(0);
		return;
	}

	for (const c::config::AudioDeviceData& device : devices)
		addItem(u::gui::removeFltkChars(device.name), device.index);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geTabAudio::geChannelMenu::geChannelMenu(const char* l)
: geChoice(l, LABEL_WIDTH)
{
}

/* -------------------------------------------------------------------------- */

int geTabAudio::geChannelMenu::getChannelsCount() const
{
	return getSelectedId() < STEREO_OFFSET ? 1 : 2;
}

int geTabAudio::geChannelMenu::getChannelsStart() const
{
	return getChannelsCount() == 1 ? getSelectedId() : getSelectedId() - STEREO_OFFSET;
}

/* -------------------------------------------------------------------------- */

void geTabAudio::geChannelMenu::rebuild(const c::config::AudioDeviceData& data)
{
	clear();

	if (data.index == -1)
	{
		addItem(g_ui.getI18Text(LangMap::COMMON_NONE), 0);
		showItem(0);
		return;
	}

	/* Single channel choice (mono) is available only for input devices. Output
	devices are always stereo. */

	if (data.type == c::config::DeviceType::INPUT)
		for (int i = 0; i < data.channelsMax; i++)
			addItem(std::to_string(i + 1), i);

	/* Dirty trick for stereo channels: they start at STEREO_OFFSET. Also,
	what if channelsMax > 2? Only channel pairs are allowed at the moment. */

	if (data.channelsMax > 1)
		for (int i = 0; i < data.channelsMax; i += 2)
			addItem(fmt::format("{}-{}", i + 1, i + 2), i + STEREO_OFFSET);

	if (data.channelsCount == 1)
		showItem(data.channelsStart);
	else
		showItem(data.channelsStart + STEREO_OFFSET);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geTabAudio::geTabAudio(geompp::Rect<int> bounds)
: Fl_Group(bounds.x, bounds.y, bounds.w, bounds.h, g_ui.getI18Text(LangMap::CONFIG_AUDIO_TITLE))
{
	end();

	geFlex* body = new geFlex(bounds.reduced(G_GUI_OUTER_MARGIN), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		m_api = new geChoice(g_ui.getI18Text(LangMap::CONFIG_AUDIO_SYSTEM), LABEL_WIDTH);

		geFlex* line1 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			m_bufferSize = new geChoice(g_ui.getI18Text(LangMap::CONFIG_AUDIO_BUFFERSIZE), LABEL_WIDTH);
			m_sampleRate = new geChoice(g_ui.getI18Text(LangMap::CONFIG_AUDIO_SAMPLERATE), LABEL_WIDTH);

			line1->add(m_bufferSize, 180);
			line1->add(m_sampleRate, 180);
			line1->end();
		}

		m_sounddevOut = new geDeviceMenu(g_ui.getI18Text(LangMap::CONFIG_AUDIO_OUTPUTDEVICE));

		geFlex* line2 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			m_channelsOut = new geChannelMenu(g_ui.getI18Text(LangMap::CONFIG_AUDIO_OUTPUTCHANNELS));
			m_limitOutput = new geCheck(x() + 177, y() + 93, 100, 20, g_ui.getI18Text(LangMap::CONFIG_AUDIO_LIMITOUTPUT));

			line2->add(m_channelsOut, 180);
			line2->add(m_limitOutput);
			line2->end();
		}

		geFlex* line3 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			m_sounddevIn = new geDeviceMenu(g_ui.getI18Text(LangMap::CONFIG_AUDIO_INPUTDEVICE));
			m_enableIn   = new geCheck(0, 0, 0, 0);

			line3->add(m_sounddevIn);
			line3->add(m_enableIn, 12);
			line3->end();
		}

		geFlex* line4 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			m_channelsIn      = new geChannelMenu(g_ui.getI18Text(LangMap::CONFIG_AUDIO_INPUTCHANNELS));
			m_recTriggerLevel = new geInput(g_ui.getI18Text(LangMap::CONFIG_AUDIO_RECTHRESHOLD), 120);

			line4->add(m_channelsIn, 180);
			line4->add(m_recTriggerLevel, 180);
			line4->end();
		}

		geFlex* col1 = new geFlex(Direction::VERTICAL);
		{
			geFlex* line5 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
			{
				m_applyBtn = new geTextButton(g_ui.getI18Text(LangMap::COMMON_APPLY));

				line5->add(new geBox());
				line5->add(m_applyBtn, 80);
				line5->add(new geBox());
				line5->end();
			}

			col1->add(new geBox());
			col1->add(line5, G_GUI_UNIT);
			col1->add(new geBox());
			col1->end();
		}

		m_rsmpQuality = new geChoice(g_ui.getI18Text(LangMap::CONFIG_AUDIO_RESAMPLING), LABEL_WIDTH);

		body->add(m_api, 20);
		body->add(line1, 20);
		body->add(m_sounddevOut, 20);
		body->add(line2, 20);
		body->add(line3, 20);
		body->add(line4, 20);
		body->add(m_rsmpQuality, 20);
		body->add(col1);
		body->end();
	}

	add(body);
	resizable(body);

	m_api->onChange = [this](ID id) {
		m_data.api = static_cast<RtAudio::Api>(id);
		deactivateAll();
		c::config::changeAudioAPI(static_cast<RtAudio::Api>(id));
		rebuild(c::config::getAudioData());
		activateAll();
	};

	m_sampleRate->onChange = [this](ID id) { m_data.sampleRate = id; };

	m_sounddevOut->onChange = [this](ID id) {
		m_data.setOutputDevice(id);
		refreshDevOutProperties();
		// Also refresh channels out info
		m_data.outputDevice.channelsCount = m_channelsOut->getChannelsCount();
		m_data.outputDevice.channelsStart = m_channelsOut->getChannelsStart();
	};

	m_sounddevIn->onChange = [this](ID id) {
		m_data.setInputDevice(id);
		refreshDevInProperties();
		// Also refresh channels in info
		m_data.inputDevice.channelsCount = m_channelsIn->getChannelsCount();
		m_data.inputDevice.channelsStart = m_channelsIn->getChannelsStart();
	};

	m_enableIn->copy_tooltip(g_ui.getI18Text(LangMap::CONFIG_AUDIO_ENABLEINPUT));
	m_enableIn->onChange = [this](bool b) { m_data.setInputDevice(b ? 0 : -1); refreshDevInProperties(); };

	m_channelsOut->onChange = [this](ID) {
		m_data.outputDevice.channelsCount = m_channelsOut->getChannelsCount();
		m_data.outputDevice.channelsStart = m_channelsOut->getChannelsStart();
	};

	m_channelsIn->onChange = [this](ID) {
		m_data.inputDevice.channelsCount = m_channelsIn->getChannelsCount();
		m_data.inputDevice.channelsStart = m_channelsIn->getChannelsStart();
	};

	m_limitOutput->onChange = [this](bool v) { m_data.limitOutput = v; };

	m_bufferSize->addItem("8", 8);
	m_bufferSize->addItem("16", 16);
	m_bufferSize->addItem("32", 32);
	m_bufferSize->addItem("64", 64);
	m_bufferSize->addItem("128", 128);
	m_bufferSize->addItem("256", 256);
	m_bufferSize->addItem("512", 512);
	m_bufferSize->addItem("1024", 1024);
	m_bufferSize->addItem("2048", 2048);
	m_bufferSize->addItem("4096", 4096);
	m_bufferSize->onChange = [this](ID id) { m_data.bufferSize = id; };

	m_rsmpQuality->addItem(g_ui.getI18Text(LangMap::CONFIG_AUDIO_RESAMPLING_SINCBEST), 0);
	m_rsmpQuality->addItem(g_ui.getI18Text(LangMap::CONFIG_AUDIO_RESAMPLING_SINCMEDIUM), 1);
	m_rsmpQuality->addItem(g_ui.getI18Text(LangMap::CONFIG_AUDIO_RESAMPLING_SINCBASIC), 2);
	m_rsmpQuality->addItem(g_ui.getI18Text(LangMap::CONFIG_AUDIO_RESAMPLING_ZEROORDER), 3);
	m_rsmpQuality->addItem(g_ui.getI18Text(LangMap::CONFIG_AUDIO_RESAMPLING_LINEAR), 4);

	m_rsmpQuality->onChange = [this](ID id) { m_data.resampleQuality = id; };

	m_recTriggerLevel->onChange = [this](const std::string& s) { m_data.recTriggerLevel = std::stof(s); };

	m_applyBtn->onClick = [this]() { c::config::save(m_data); };

	rebuild(c::config::getAudioData());
}

/* -------------------------------------------------------------------------- */

void geTabAudio::rebuild(const c::config::AudioData& data)
{
	m_data = data;

	activateAll();

	for (const auto& [key, value] : m_data.apis)
		m_api->addItem(value.c_str(), key);
	m_api->showItem(m_data.api);

	m_bufferSize->showItem(m_data.bufferSize);
	if (m_data.api == RtAudio::Api::UNIX_JACK)
		m_bufferSize->deactivate();
	else
		m_bufferSize->activate();

	m_sounddevOut->rebuild(m_data.outputDevices);
	m_sounddevOut->showItem(m_data.outputDevice.index);

	m_sounddevIn->rebuild(m_data.inputDevices);
	m_sounddevIn->showItem(m_data.inputDevice.index);

	m_enableIn->value(m_data.inputDevice.index != -1);

	m_limitOutput->value(m_data.limitOutput);

	m_rsmpQuality->showItem(m_data.resampleQuality);

	m_recTriggerLevel->setValue(fmt::format("{:.1f}", m_data.recTriggerLevel));

	refreshDevOutProperties();
	refreshDevInProperties();
}

/* -------------------------------------------------------------------------- */

void geTabAudio::refreshDevOutProperties()
{
	for (unsigned int sampleRate : m_data.outputDevice.sampleRates)
		m_sampleRate->addItem(std::to_string(sampleRate), sampleRate);
	m_sampleRate->showItem(m_data.sampleRate);

	m_channelsOut->rebuild(m_data.outputDevice);
}

/* -------------------------------------------------------------------------- */

void geTabAudio::refreshDevInProperties()
{
	if (m_data.inputDevice.index != -1)
	{
		m_channelsIn->rebuild(m_data.inputDevice);
		m_sounddevIn->activate();
		m_channelsIn->activate();
		m_recTriggerLevel->activate();
	}
	else
	{
		m_sounddevIn->deactivate();
		m_channelsIn->deactivate();
		m_recTriggerLevel->deactivate();
	}
}

/* -------------------------------------------------------------------------- */

void geTabAudio::deactivateAll()
{
	m_bufferSize->deactivate();
	m_limitOutput->deactivate();
	m_sounddevOut->deactivate();
	m_channelsOut->deactivate();
	m_sampleRate->deactivate();
	m_sounddevIn->deactivate();
	m_channelsIn->deactivate();
	m_recTriggerLevel->deactivate();
	m_rsmpQuality->deactivate();
}

/* -------------------------------------------------------------------------- */

void geTabAudio::activateAll()
{
	m_bufferSize->activate();
	m_limitOutput->activate();
	m_sounddevOut->activate();
	m_channelsOut->activate();
	m_sampleRate->activate();
	m_sounddevIn->activate();
	m_channelsIn->activate();
	m_recTriggerLevel->activate();
	m_rsmpQuality->activate();
}

/* -------------------------------------------------------------------------- */

void geTabAudio::save()
{
	c::config::save(m_data);
}
} // namespace giada::v