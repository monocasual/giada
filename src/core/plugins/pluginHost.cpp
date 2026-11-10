/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/core/plugins/pluginHost.h"
#include "src/core/channels/channel.h"
#include "src/core/const.h"
#include "src/core/model/assets.h"
#include "src/core/model/plugin.h"
#include "src/core/plugins/pluginManager.h"
#include "src/deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include "src/deps/mcl-utils/src/container.hpp"
#include "src/utils/log.h"
#include <cassert>
#include <cstddef>
#include <memory>

namespace utils = mcl::utils;

namespace giada::m
{
PluginHost::PluginHost(model::RealtimeModel& m)
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

void PluginHost::processStack(
    const model::Channel::Rendering&         rendering,
    const std::vector<ID>&                   pluginIds,
    const model::Assets&                     assets,
    const juce::AudioPlayHead::PositionInfo& position,
    const juce::MidiBuffer*                  events)
{
	if (pluginIds.empty())
		return;

	const mcl::AudioBuffer& buffer = rendering.getAudioBuffer();

	assert(buffer.countFrames() == m_audioBuffer.getNumSamples());
	assert(buffer.countChannels() == m_audioBuffer.getNumChannels());

	/* Seed the internal working buffer with this channel's current audio. */

	giadaToJuceTempBuf(buffer);

	/* Keep one mutable MIDI stream for the stack so MIDI changes from one
	plug-in are visible to the next. */

	juce::MidiBuffer chainEvents;
	if (events != nullptr)
		chainEvents = *events;

	for (const ID pluginId : pluginIds)
	{
		const auto plugin = assets.plugins.find(pluginId);
		assert(plugin != nullptr);

		if (!plugin->valid || plugin->isSuspended() || plugin->isBypassed())
			continue;

		plugin->setPlayHeadPosition(position);
		processPlugin(*plugin, chainEvents);
	}

	/* Write the final chain result back to the channel's rendering buffer. */

	rendering.copyToAudioBufferFrom(
	    m_audioBuffer, m_audioBuffer.getNumSamples(), 0, 0);
}

/* -------------------------------------------------------------------------- */

void PluginHost::freePlugin(ID pluginId, ID channelId)
{
	m_model.writeDocumentAndAssets(model::SwapType::HARD,
	    [pluginId, channelId](model::Document& doc, model::Assets& assets)
	{
		utils::container::removeIf(doc.tracks.getChannel(channelId).plugins, [pluginId](ID id)
		{ return id == pluginId; });

		assets.plugins.remove(pluginId);
	});
}

/* -------------------------------------------------------------------------- */

void PluginHost::freePlugins(ID channelId)
{
	m_model.writeDocumentAndAssets(model::SwapType::HARD,
	    [channelId](model::Document& doc, model::Assets& assets)
	{
		for (const ID pluginId : doc.tracks.getChannel(channelId).plugins)
			assets.plugins.remove(pluginId);
		doc.tracks.getChannel(channelId).plugins.clear();
	});
}

/* -------------------------------------------------------------------------- */

void PluginHost::swapPlugins(ID pluginId1, ID pluginId2, ID channelId)
{
	m_model.writeDocument(model::SwapType::HARD, [pluginId1, pluginId2, channelId](model::Document& doc)
	{
		auto&             plugins = doc.tracks.getChannel(channelId).plugins;
		const std::size_t index1  = utils::container::indexOfIf(plugins, [pluginId1](ID id)
		 { return id == pluginId1; });
		const std::size_t index2  = utils::container::indexOfIf(plugins, [pluginId2](ID id)
		 { return id == pluginId2; });
		std::swap(plugins.at(index1), plugins.at(index2));
	});
}

/* -------------------------------------------------------------------------- */

void PluginHost::freeAllPlugins()
{
	m_model.writeAssets(model::SwapType::HARD, [](model::Assets& assets)
	{
		assets.plugins.clear();
	});
}

/* -------------------------------------------------------------------------- */

void PluginHost::setPluginParameter(ID pluginId, int paramIndex, float value)
{
	m_model.getCurrentAssets()->plugins.get(pluginId)->setParameter(paramIndex, value);
}

/* -------------------------------------------------------------------------- */

void PluginHost::setPluginProgram(ID pluginId, int programIndex)
{
	m_model.getCurrentAssets()->plugins.get(pluginId)->setCurrentProgram(programIndex);
}

/* -------------------------------------------------------------------------- */

void PluginHost::toggleBypass(ID pluginId)
{
	auto plugin = m_model.getCurrentAssets()->plugins.get(pluginId);
	plugin->setBypass(!plugin->isBypassed());
}

/* -------------------------------------------------------------------------- */

void PluginHost::giadaToJuceTempBuf(const mcl::AudioBuffer& outBuf)
{
	assert(outBuf.countChannels() == m_audioBuffer.getNumChannels());
	assert(outBuf.countFrames() == m_audioBuffer.getNumSamples());

	for (int ch = 0; ch < outBuf.countChannels(); ++ch)
	{
		const float* src = outBuf.getChannelView(ch).data();
		float*       dst = m_audioBuffer.getWritePointer(ch);

		std::copy(src, src + outBuf.countFrames(), dst);
	}
}

/* -------------------------------------------------------------------------- */

void PluginHost::processPlugin(const model::Plugin& plugin, juce::MidiBuffer& events)
{
	const model::Plugin::Buffer& output = plugin.process(m_audioBuffer, events);

	/* Preserve the existing channel-layout mapping: the next plug-in receives
	this plug-in's output in the host's working buffer. */

	for (int hostChannel = 0, pluginChannel = 0; hostChannel < m_audioBuffer.getNumChannels(); ++hostChannel)
	{
		m_audioBuffer.copyFrom(hostChannel, 0, output, pluginChannel, 0, output.getNumSamples());
		if (hostChannel < plugin.countMainOutChannels() - 1)
			++pluginChannel;
	}
}
} // namespace giada::m
