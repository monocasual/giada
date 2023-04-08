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
ConfigApi::ConfigApi(model::Model& m, KernelAudio& k)
: m_model(m)
, m_kernelAudio(k)
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
} // namespace giada::m
