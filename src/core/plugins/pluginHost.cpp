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

#include "core/plugins/pluginHost.h"
#include "core/channels/channel.h"
#include "core/const.h"
#include "core/model/model.h"
#include "core/plugins/plugin.h"
#include "core/plugins/pluginManager.h"
#include "deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include "utils/log.h"
#include "utils/vector.h"
#include <cassert>
#include <cstddef>
#include <memory>

namespace giada::m
{
PluginHost::Info::Info(const model::Sequencer& s, int sampleRate)
: m_sequencer(s)
, m_sampleRate(sampleRate)
{
}

/* -------------------------------------------------------------------------- */

juce::Optional<juce::AudioPlayHead::PositionInfo> PluginHost::Info::getPosition() const
{
	juce::AudioPlayHead::PositionInfo info;

	info.setBpm(m_sequencer.bpm);
	info.setTimeInSamples(m_sequencer.a_getCurrentFrame());
	info.setTimeInSeconds(m_sequencer.a_getCurrentSecond(m_sampleRate));
	info.setIsPlaying(m_sequencer.isRunning());

	return {info};
}

/* -------------------------------------------------------------------------- */

bool PluginHost::Info::canControlTransport()
{
	return false;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

PluginHost::PluginHost(model::Model& m)
: m_model(m)
{
}

/* -------------------------------------------------------------------------- */

void PluginHost::reset(int bufferSize)
{
	freeAllPlugins();
	setBufferSize(bufferSize);
}

/* -------------------------------------------------------------------------- */

void PluginHost::setBufferSize(int bufferSize)
{
	m_audioBuffer.setSize(G_MAX_IO_CHANS, bufferSize);
}

/* -------------------------------------------------------------------------- */

void PluginHost::processStack(mcl::AudioBuffer& outBuf, const std::vector<Plugin*>& plugins,
    juce::MidiBuffer* events)
{
	assert(outBuf.countFrames() == m_audioBuffer.getNumSamples());

	giadaToJuceTempBuf(outBuf);

	if (events == nullptr)
	{
		juce::MidiBuffer dummyEvents; // empty
		processPlugins(plugins, dummyEvents);
	}
	else
		processPlugins(plugins, *events);

	juceToGiadaOutBuf(outBuf);
}

/* -------------------------------------------------------------------------- */

const Plugin& PluginHost::addPlugin(std::unique_ptr<Plugin> p)
{
	return m_model.addPlugin(std::move(p));
}

/* -------------------------------------------------------------------------- */

void PluginHost::swapPlugin(const m::Plugin& p1, const m::Plugin& p2, std::vector<Plugin*>& plugins)
{
	std::size_t index1 = u::vector::indexOf(plugins, &p1);
	std::size_t index2 = u::vector::indexOf(plugins, &p2);
	std::swap(plugins.at(index1), plugins.at(index2));
}

/* -------------------------------------------------------------------------- */

void PluginHost::freePlugin(const m::Plugin& plugin)
{
	m_model.removeShared(plugin);
}

void PluginHost::freePlugins(const std::vector<Plugin*>& plugins)
{
	for (const Plugin* p : plugins)
		m_model.removeShared(*p);
}

/* -------------------------------------------------------------------------- */

void PluginHost::freeAllPlugins()
{
	m_model.clearShared<model::PluginPtrs>();
}

/* -------------------------------------------------------------------------- */

void PluginHost::setPluginParameter(ID pluginId, int paramIndex, float value)
{
	m_model.findPlugin(pluginId)->setParameter(paramIndex, value);
}

/* -------------------------------------------------------------------------- */

void PluginHost::setPluginProgram(ID pluginId, int programIndex)
{
	m_model.findPlugin(pluginId)->setCurrentProgram(programIndex);
}

/* -------------------------------------------------------------------------- */

void PluginHost::toggleBypass(ID pluginId)
{
	Plugin& plugin = *m_model.findPlugin(pluginId);
	plugin.setBypass(!plugin.isBypassed());
}

/* -------------------------------------------------------------------------- */

void PluginHost::giadaToJuceTempBuf(const mcl::AudioBuffer& outBuf)
{
	assert(outBuf.countChannels() == m_audioBuffer.getNumChannels());

	using namespace juce;
	using Format = AudioData::Format<AudioData::Float32, AudioData::BigEndian>;

	AudioData::deinterleaveSamples(
	    AudioData::InterleavedSource<Format>{outBuf[0], outBuf.countChannels()},
	    AudioData::NonInterleavedDest<Format>{m_audioBuffer.getArrayOfWritePointers(), m_audioBuffer.getNumChannels()},
	    outBuf.countFrames());
}

void PluginHost::juceToGiadaOutBuf(mcl::AudioBuffer& outBuf) const
{
	assert(outBuf.countChannels() == m_audioBuffer.getNumChannels());

	using namespace juce;
	using Format = AudioData::Format<AudioData::Float32, AudioData::BigEndian>;

	AudioData::interleaveSamples(
	    AudioData::NonInterleavedSource<Format>{m_audioBuffer.getArrayOfReadPointers(), m_audioBuffer.getNumChannels()},
	    AudioData::InterleavedDest<Format>{outBuf[0], outBuf.countChannels()},
	    outBuf.countFrames());
}

/* -------------------------------------------------------------------------- */

void PluginHost::processPlugins(const std::vector<Plugin*>& plugins, juce::MidiBuffer& events)
{
	for (Plugin* p : plugins)
	{
		if (!p->valid || p->isSuspended() || p->isBypassed())
			continue;
		processPlugin(p, events);
	}
	events.clear();
}

/* -------------------------------------------------------------------------- */

void PluginHost::processPlugin(Plugin* p, const juce::MidiBuffer& events)
{
	const Plugin::Buffer& pluginBuffer = p->process(m_audioBuffer, events);
	const bool            isInstrument = p->isInstrument();

	/* Merge the plugin buffer back into the local one. Special care is needed
	if audio channels mismatch. */

	for (int i = 0, j = 0; i < m_audioBuffer.getNumChannels(); i++)
	{
		/* If instrument (i.e. a plug-in that accepts MIDI and produces audio 
		out of it), SUM the local working buffer to the main one. This allows
		multiple plug-in instruments to play simultaneously on a given set of
		MIDI events. If it's a normal FX instead (!isInstrument), the local
		working buffer is simply copied over the main one. */

		if (isInstrument)
			m_audioBuffer.addFrom(i, 0, pluginBuffer, j, 0, pluginBuffer.getNumSamples());
		else
			m_audioBuffer.copyFrom(i, 0, pluginBuffer, j, 0, pluginBuffer.getNumSamples());
		if (i < p->countMainOutChannels() - 1)
			j++;
	}
}
} // namespace giada::m