/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifdef WITH_VST

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
PluginHost::Info::Info(const Sequencer& s, int sampleRate)
: m_sequencer(s)
, m_sampleRate(sampleRate)
{
}

/* -------------------------------------------------------------------------- */

bool PluginHost::Info::getCurrentPosition(CurrentPositionInfo& result)
{
	result.bpm           = m_sequencer.getBpm();
	result.timeInSamples = m_sequencer.getCurrentFrame();
	result.timeInSeconds = m_sequencer.getCurrentSecond(m_sampleRate);
	result.isPlaying     = m_sequencer.isRunning();

	return true;
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
	m_audioBuffer.setSize(G_MAX_IO_CHANS, bufferSize);
}

/* -------------------------------------------------------------------------- */

void PluginHost::processStack(mcl::AudioBuffer& outBuf, const std::vector<Plugin*>& plugins,
    juce::MidiBuffer* events)
{
	assert(outBuf.countFrames() == m_audioBuffer.getNumSamples());

	/* If events are null: Audio stack processing (master in, master out or
	sample channels. No need for MIDI events. 
	If events are not null: MIDI stack (MIDI channels). MIDI channels must not 
	process the current buffer: give them an empty and clean one. */

	if (events == nullptr)
	{
		giadaToJuceTempBuf(outBuf);
		juce::MidiBuffer dummyEvents; // empty
		processPlugins(plugins, dummyEvents);
	}
	else
	{
		m_audioBuffer.clear();
		processPlugins(plugins, *events);
	}
	juceToGiadaOutBuf(outBuf);
}

/* -------------------------------------------------------------------------- */

const Plugin& PluginHost::addPlugin(std::unique_ptr<Plugin> p)
{
	m_model.addShared(std::move(p));
	return m_model.backShared<Plugin>();
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
	m_model.findShared<Plugin>(pluginId)->setParameter(paramIndex, value);
}

/* -------------------------------------------------------------------------- */

void PluginHost::setPluginProgram(ID pluginId, int programIndex)
{
	m_model.findShared<Plugin>(pluginId)->setCurrentProgram(programIndex);
}

/* -------------------------------------------------------------------------- */

void PluginHost::toggleBypass(ID pluginId)
{
	Plugin& plugin = *m_model.findShared<Plugin>(pluginId);
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
		p->process(m_audioBuffer, events);
	}
	events.clear();
}
} // namespace giada::m

#endif // #ifdef WITH_VST
