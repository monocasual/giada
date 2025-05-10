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

extern giada::v::Ui* g_ui;

namespace giada::v
{
geTabAudio::geDeviceMenu::geDeviceMenu()
: geChoice()
{
}

/* -------------------------------------------------------------------------- */

void geTabAudio::geDeviceMenu::rebuild(const std::vector<c::config::AudioDeviceData>& devices)
{
	clear();

	if (devices.size() == 0)
	{
		addItem(g_ui->getI18Text(LangMap::CONFIG_AUDIO_NODEVICESFOUND), 0);
		showFirstItem();
		return;
	}

	for (const c::config::AudioDeviceData& device : devices)
		addItem(u::gui::removeFltkChars(device.name), device.id);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geTabAudio::geChannelMenu::geChannelMenu()
: geChoice()
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

	if (data.id == 0 || data.channelsMax == 0)
	{
		addItem(g_ui->getI18Text(LangMap::COMMON_NONE), 0);
		showFirstItem();
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

	if (data.selectedChannelsCount == 0) // First time you choose a device, so no channels selected yet: just show first item
		showFirstItem();
	else if (data.selectedChannelsCount == 1)
		showItem(data.selectedChannelsStart);
	else if (data.selectedChannelsCount == 2)
		showItem(data.selectedChannelsStart + STEREO_OFFSET);
	else
		assert(false);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geTabAudio::geTabAudio(geompp::Rect<int> bounds)
: Fl_Group(bounds.x, bounds.y, bounds.w, bounds.h, g_ui->getI18Text(LangMap::CONFIG_AUDIO_TITLE))
{
	end();

	const int LABEL_WIDTH = 120;

	geFlex* body = new geFlex(bounds.reduced(G_GUI_OUTER_MARGIN), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		geFlex* line0 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			m_api = new geChoice();

			line0->addWidget(new geBox(g_ui->getI18Text(LangMap::CONFIG_AUDIO_SYSTEM), FL_ALIGN_RIGHT), LABEL_WIDTH);
			line0->addWidget(m_api);
			line0->end();
		}

		geFlex* line1 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			m_bufferSize = new geChoice();

			line1->addWidget(new geBox(g_ui->getI18Text(LangMap::CONFIG_AUDIO_BUFFERSIZE), FL_ALIGN_RIGHT), LABEL_WIDTH);
			line1->addWidget(m_bufferSize, 60);
			line1->end();
		}

		geFlex* line2 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			m_sampleRate = new geChoice();

			line2->addWidget(new geBox(g_ui->getI18Text(LangMap::CONFIG_AUDIO_SAMPLERATE), FL_ALIGN_RIGHT), LABEL_WIDTH);
			line2->addWidget(m_sampleRate, 60);
			line2->end();
		}

		geFlex* line3 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			m_sounddevOut = new geDeviceMenu();

			line3->addWidget(new geBox(g_ui->getI18Text(LangMap::CONFIG_AUDIO_OUTPUTDEVICE), FL_ALIGN_RIGHT), LABEL_WIDTH);
			line3->addWidget(m_sounddevOut);
			line3->end();
		}

		geFlex* line4 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			m_channelsOut = new geChannelMenu();
			m_limitOutput = new geCheck();

			line4->addWidget(new geBox(g_ui->getI18Text(LangMap::CONFIG_AUDIO_OUTPUTCHANNELS), FL_ALIGN_RIGHT), LABEL_WIDTH);
			line4->addWidget(m_channelsOut, 60);
			line4->addWidget(m_limitOutput, 12);
			line4->addWidget(new geBox(g_ui->getI18Text(LangMap::CONFIG_AUDIO_LIMITOUTPUT), FL_ALIGN_LEFT), LABEL_WIDTH);
			line4->end();
		}

		geFlex* line5 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			m_sounddevIn = new geDeviceMenu();
			m_enableIn   = new geCheck();

			line5->addWidget(new geBox(g_ui->getI18Text(LangMap::CONFIG_AUDIO_INPUTDEVICE), FL_ALIGN_RIGHT), LABEL_WIDTH);
			line5->addWidget(m_sounddevIn);
			line5->addWidget(m_enableIn, 12);
			line5->end();
		}

		geFlex* line6 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			m_channelsIn      = new geChannelMenu();
			m_recTriggerLevel = new geInput();

			line6->addWidget(new geBox(g_ui->getI18Text(LangMap::CONFIG_AUDIO_INPUTCHANNELS), FL_ALIGN_RIGHT), LABEL_WIDTH);
			line6->addWidget(m_channelsIn, 60);
			line6->addWidget(new geBox(g_ui->getI18Text(LangMap::CONFIG_AUDIO_RECTHRESHOLD), FL_ALIGN_RIGHT), LABEL_WIDTH);
			line6->addWidget(m_recTriggerLevel, 60);
			line6->end();
		}

		geFlex* line7 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			m_rsmpQuality = new geChoice();

			line7->addWidget(new geBox(g_ui->getI18Text(LangMap::CONFIG_AUDIO_RESAMPLING), FL_ALIGN_RIGHT), LABEL_WIDTH);
			line7->addWidget(m_rsmpQuality);
			line7->end();
		}

		geFlex* line8 = new geFlex(Direction::HORIZONTAL);
		{
			m_applyBtn = new geTextButton(g_ui->getI18Text(LangMap::COMMON_APPLY));

			line8->addWidget(new geBox());
			line8->addWidget(m_applyBtn, 80);
			line8->addWidget(new geBox());
			line8->end();
		}

		body->addWidget(line0, 20);
		body->addWidget(line1, 20);
		body->addWidget(line2, 20);
		body->addWidget(line3, 20);
		body->addWidget(line4, 20);
		body->addWidget(line5, 20);
		body->addWidget(line6, 20);
		body->addWidget(line7, 20);
		body->addWidget(new geBox());
		body->addWidget(line8, 20);
		body->addWidget(new geBox());
		body->end();
	}

	add(body);
	resizable(body);

	m_api->onChange = [this](ID id)
	{
		m_data.selectedApi = static_cast<RtAudio::Api>(id);
		deactivateAll();
		c::config::changeAudioAPI(static_cast<RtAudio::Api>(id));
		rebuild(c::config::getAudioData());
		activateAll();
	};

	m_sampleRate->onChange = [this](ID id)
	{ m_data.selectedSampleRate = id; };

	m_sounddevOut->onChange = [this](ID id)
	{
		m_data.setOutputDevice(id);
		refreshDevOutProperties();
	};

	m_sounddevIn->onChange = [this](ID id)
	{
		m_data.setInputDevice(id);
		refreshDevInProperties();
	};

	m_enableIn->copy_tooltip(g_ui->getI18Text(LangMap::CONFIG_AUDIO_ENABLEINPUT));
	m_enableIn->onChange = [this](bool b)
	{
		m_data.toggleInputDevice(b);
		refreshDevInProperties();
	};

	m_channelsOut->onChange = [this](ID)
	{
		m_data.selectedOutputDevice.selectedChannelsCount = m_channelsOut->getChannelsCount();
		m_data.selectedOutputDevice.selectedChannelsStart = m_channelsOut->getChannelsStart();
	};

	m_channelsIn->onChange = [this](ID)
	{
		m_data.selectedInputDevice.selectedChannelsCount = m_channelsIn->getChannelsCount();
		m_data.selectedInputDevice.selectedChannelsStart = m_channelsIn->getChannelsStart();
	};

	m_limitOutput->onChange = [this](bool v)
	{ m_data.selectedLimitOutput = v; };

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
	m_bufferSize->onChange = [this](ID id)
	{ m_data.selectedBufferSize = id; };

	m_rsmpQuality->addItem(g_ui->getI18Text(LangMap::CONFIG_AUDIO_RESAMPLING_SINCBEST), 0);
	m_rsmpQuality->addItem(g_ui->getI18Text(LangMap::CONFIG_AUDIO_RESAMPLING_SINCMEDIUM), 1);
	m_rsmpQuality->addItem(g_ui->getI18Text(LangMap::CONFIG_AUDIO_RESAMPLING_SINCBASIC), 2);
	m_rsmpQuality->addItem(g_ui->getI18Text(LangMap::CONFIG_AUDIO_RESAMPLING_ZEROORDER), 3);
	m_rsmpQuality->addItem(g_ui->getI18Text(LangMap::CONFIG_AUDIO_RESAMPLING_LINEAR), 4);

	m_rsmpQuality->onChange = [this](ID id)
	{ m_data.selectedResampleQuality = id; };

	m_recTriggerLevel->onChange = [this](const std::string& s)
	{ m_data.selectedRecTriggerLevel = std::stof(s); };

	m_applyBtn->onClick = [this]()
	{ c::config::apply(m_data); };

	rebuild(c::config::getAudioData());
}

/* -------------------------------------------------------------------------- */

void geTabAudio::rebuild(const c::config::AudioData& data)
{
	m_data = data;

	activateAll();

	for (const auto& [key, value] : m_data.availableApis)
		m_api->addItem(value.c_str(), key);
	m_api->showItem(m_data.selectedApi);

	m_bufferSize->showItem(m_data.selectedBufferSize);
	if (m_data.selectedApi == RtAudio::Api::UNIX_JACK)
		m_bufferSize->deactivate();
	else
		m_bufferSize->activate();

	m_sounddevOut->rebuild(m_data.availableOutputDevices);
	m_sounddevOut->showItem(m_data.selectedOutputDevice.id);

	m_sounddevIn->rebuild(m_data.availableInputDevices);
	if (m_data.selectedInputDevice.id != 0)
		m_sounddevIn->showItem(m_data.selectedInputDevice.id);

	m_enableIn->value(m_data.selectedInputDevice.id != 0);

	m_limitOutput->value(m_data.selectedLimitOutput);

	m_rsmpQuality->showItem(m_data.selectedResampleQuality);

	m_recTriggerLevel->setValue(fmt::format("{:.1f}", m_data.selectedRecTriggerLevel));

	refreshDevOutProperties();
	refreshDevInProperties();
}

/* -------------------------------------------------------------------------- */

void geTabAudio::refreshDevOutProperties()
{
	m_sampleRate->clear();

	if (m_data.selectedOutputDevice.sampleRates.size() == 0)
	{
		m_sampleRate->addItem(g_ui->getI18Text(LangMap::COMMON_NONE), 0);
		m_sampleRate->showFirstItem();
		m_sampleRate->deactivate();
	}
	else
	{
		for (unsigned int sampleRate : m_data.selectedOutputDevice.sampleRates)
			m_sampleRate->addItem(std::to_string(sampleRate), sampleRate);
		m_sampleRate->showItem(m_data.selectedSampleRate);
		m_sampleRate->activate();
	}

	m_channelsOut->rebuild(m_data.selectedOutputDevice);

	// Also refresh channels out info
	m_data.selectedOutputDevice.selectedChannelsCount = m_channelsOut->getChannelsCount();
	m_data.selectedOutputDevice.selectedChannelsStart = m_channelsOut->getChannelsStart();
}

/* -------------------------------------------------------------------------- */

void geTabAudio::refreshDevInProperties()
{
	if (m_data.selectedInputDevice.id != 0)
	{
		m_channelsIn->rebuild(m_data.selectedInputDevice);
		m_sounddevIn->activate();
		m_channelsIn->activate();
		m_recTriggerLevel->activate();
		// Also refresh channels in info
		m_data.selectedInputDevice.selectedChannelsCount = m_channelsIn->getChannelsCount();
		m_data.selectedInputDevice.selectedChannelsStart = m_channelsIn->getChannelsStart();
	}
	else
	{
		m_sounddevIn->deactivate();
		m_sounddevIn->showFirstItem();
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
} // namespace giada::v