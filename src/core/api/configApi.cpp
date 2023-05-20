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

#include "core/api/configApi.h"
#include "core/kernelAudio.h"
#include "core/model/model.h"

namespace giada::m
{
ConfigApi::ConfigApi(model::Model& m, KernelAudio& ka, KernelMidi& km, MidiMapper<KernelMidi>& mm)
: m_model(m)
, m_kernelAudio(ka)
, m_kernelMidi(km)
, m_midiMapper(mm)
{
}

/* -------------------------------------------------------------------------- */

bool ConfigApi::audio_hasAPI(RtAudio::Api api) const
{
	return m_kernelAudio.hasAPI(api);
}

/* -------------------------------------------------------------------------- */

RtAudio::Api ConfigApi::audio_getAPI() const
{
	return m_kernelAudio.getAPI();
}

/* -------------------------------------------------------------------------- */

std::vector<KernelAudio::Device> ConfigApi::audio_getAvailableDevices() const
{
	return m_kernelAudio.getAvailableDevices();
}

/* -------------------------------------------------------------------------- */

KernelAudio::Device ConfigApi::audio_getCurrentOutDevice() const
{
	return m_kernelAudio.getCurrentOutDevice();
}

/* -------------------------------------------------------------------------- */

KernelAudio::Device ConfigApi::audio_getCurrentInDevice() const
{
	return m_kernelAudio.getCurrentInDevice();
}

/* -------------------------------------------------------------------------- */

bool               ConfigApi::audio_isLimitOutput() const { return m_kernelAudio.isLimitOutput(); }
float              ConfigApi::audio_getRecTriggerLevel() const { return m_kernelAudio.getRecTriggerLevel(); }
Resampler::Quality ConfigApi::audio_getResamplerQuality() const { return m_kernelAudio.getResamplerQuality(); }
int                ConfigApi::audio_getSampleRate() const { return m_kernelAudio.getSampleRate(); }
int                ConfigApi::audio_getBufferSize() const { return m_kernelAudio.getBufferSize(); }

/* -------------------------------------------------------------------------- */

void ConfigApi::audio_setAPI(RtAudio::Api api)
{
	m_kernelAudio.setAPI(api);
}

/* -------------------------------------------------------------------------- */

bool ConfigApi::audio_openStream(
    const model::KernelAudio::Device& out,
    const model::KernelAudio::Device& in,
    unsigned int                      sampleRate,
    unsigned int                      bufferSize)
{
	bool res = m_kernelAudio.openStream(out, in, sampleRate, bufferSize);
	if (!res)
		return false;
	m_kernelAudio.startStream();
	return true;
}

/* -------------------------------------------------------------------------- */

void ConfigApi::audio_storeData(bool limitOutput, Resampler::Quality rsmpQuality, float recTriggerLevel)
{
	model::KernelAudio& kernelAudio = m_model.get().kernelAudio;

	kernelAudio.limitOutput     = limitOutput;
	kernelAudio.rsmpQuality     = rsmpQuality;
	kernelAudio.recTriggerLevel = recTriggerLevel;

	m_model.swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

bool ConfigApi::midi_hasAPI(RtMidi::Api api) const
{
	return m_kernelMidi.hasAPI(api);
}

/* -------------------------------------------------------------------------- */

RtMidi::Api ConfigApi::midi_getAPI() const
{
	return m_kernelMidi.getAPI();
}

/* -------------------------------------------------------------------------- */

int ConfigApi::midi_getSyncMode() const
{
	return m_kernelMidi.getSyncMode();
}

/* -------------------------------------------------------------------------- */

int ConfigApi::midi_getCurrentOutPort() const
{
	return m_kernelMidi.getCurrentOutPort();
}

int ConfigApi::midi_getCurrentInPort() const
{
	return m_kernelMidi.getCurrentInPort();
}

/* -------------------------------------------------------------------------- */

std::vector<std::string> ConfigApi::midi_getOutPorts() const
{
	return m_kernelMidi.getOutPorts();
}

std::vector<std::string> ConfigApi::midi_getInPorts() const
{
	return m_kernelMidi.getInPorts();
}

/* -------------------------------------------------------------------------- */

const std::vector<std::string>& ConfigApi::midi_getMidiMapFilesFound() const
{
	return m_midiMapper.getMapFilesFound();
}

/* -------------------------------------------------------------------------- */

bool ConfigApi::midi_setAPI(RtMidi::Api api)
{
	return m_kernelMidi.setAPI(api);
}

/* -------------------------------------------------------------------------- */

KernelMidi::Result ConfigApi::midi_openOutPort(int out)
{
	return m_kernelMidi.openOutPort(out);
}

KernelMidi::Result ConfigApi::midi_openInPort(int in)
{
	return m_kernelMidi.openInPort(in);
}

/* -------------------------------------------------------------------------- */

void ConfigApi::midi_storeData(int syncMode)
{
	m_model.get().kernelMidi.sync = syncMode;
	m_model.swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

const model::Behaviors& ConfigApi::behaviors_getData() const
{
	return m_model.get().behaviors;
}

/* -------------------------------------------------------------------------- */

void ConfigApi::behaviors_storeData(const model::Behaviors& data)
{
	m_model.get().behaviors = data;
	m_model.swap(model::SwapType::NONE);
}
} // namespace giada::m
