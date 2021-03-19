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

#ifdef WITH_VST

#include "core/plugins/pluginHost.h"
#include "core/channels/channel.h"
#include "core/const.h"
#include "core/model/model.h"
#include "core/plugins/plugin.h"
#include "core/plugins/pluginManager.h"
#include "utils/log.h"
#include "utils/vector.h"
#include <cassert>

namespace giada::m::pluginHost
{
namespace
{
juce::MessageManager*    messageManager_;
juce::AudioBuffer<float> audioBuffer_;
ID                       pluginId_;

/* -------------------------------------------------------------------------- */

void giadaToJuceTempBuf_(const AudioBuffer& outBuf)
{
	for (int i = 0; i < outBuf.countFrames(); i++)
		for (int j = 0; j < outBuf.countChannels(); j++)
			audioBuffer_.setSample(j, i, outBuf[i][j]);
}

/* juceToGiadaOutBuf_
Converts buffer from Juce to Giada. A note for the future: if we overwrite (=) 
(as we do now) it's SEND, if we add (+) it's INSERT. */

void juceToGiadaOutBuf_(AudioBuffer& outBuf)
{
	for (int i = 0; i < outBuf.countFrames(); i++)
		for (int j = 0; j < outBuf.countChannels(); j++)
			outBuf[i][j] = audioBuffer_.getSample(j, i);
}

/* -------------------------------------------------------------------------- */

void processPlugins_(const std::vector<Plugin*>& plugins, juce::MidiBuffer& events)
{
	for (Plugin* p : plugins)
	{
		if (!p->valid || p->isSuspended() || p->isBypassed())
			continue;
		p->process(audioBuffer_, events);
		events.clear();
	}
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void close()
{
	messageManager_->deleteInstance();
	model::clear<model::PluginPtrs>();
}

/* -------------------------------------------------------------------------- */

void init(int buffersize)
{
	messageManager_ = juce::MessageManager::getInstance();
	audioBuffer_.setSize(G_MAX_IO_CHANS, buffersize);
	pluginId_ = 0;
}

/* -------------------------------------------------------------------------- */

void processStack(AudioBuffer& outBuf, const std::vector<Plugin*>& plugins,
    juce::MidiBuffer* events)
{
	assert(outBuf.countFrames() == audioBuffer_.getNumSamples());

	/* If events are null: Audio stack processing (master in, master out or
	sample channels. No need for MIDI events. 
	If events are not null: MIDI stack (MIDI channels). MIDI channels must not 
	process the current buffer: give them an empty and clean one. */

	if (events == nullptr)
	{
		giadaToJuceTempBuf_(outBuf);
		juce::MidiBuffer dummyEvents; // empty
		processPlugins_(plugins, dummyEvents);
	}
	else
	{
		audioBuffer_.clear();
		processPlugins_(plugins, *events);
	}
	juceToGiadaOutBuf_(outBuf);
}

/* -------------------------------------------------------------------------- */

void addPlugin(std::unique_ptr<Plugin> p, ID channelId)
{
	model::add(std::move(p));

	const Plugin& pluginRef = model::back<Plugin>();

	/* TODO - unfortunately JUCE wants mutable plugin objects due to the
	presence of the non-const processBlock() method. Why not const_casting
	only in the Plugin class? */
	model::get().getChannel(channelId).plugins.push_back(const_cast<Plugin*>(&pluginRef));
	model::swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void swapPlugin(const m::Plugin& p1, const m::Plugin& p2, ID channelId)
{
	std::vector<m::Plugin*>& pvec   = model::get().getChannel(channelId).plugins;
	std::size_t              index1 = u::vector::indexOf(pvec, &p1);
	std::size_t              index2 = u::vector::indexOf(pvec, &p2);
	std::swap(pvec.at(index1), pvec.at(index2));

	model::swap(model::SwapType::HARD);
}

/* -------------------------------------------------------------------------- */

void freePlugin(const m::Plugin& plugin, ID channelId)
{
	u::vector::remove(model::get().getChannel(channelId).plugins, &plugin);
	model::swap(model::SwapType::HARD);
	model::remove(plugin);
}

void freePlugins(const std::vector<Plugin*>& plugins)
{
	// TODO - channels???
	for (const Plugin* p : plugins)
		model::remove(*p);
}

/* -------------------------------------------------------------------------- */

std::vector<Plugin*> clonePlugins(const std::vector<Plugin*>& plugins)
{
	std::vector<Plugin*> out;
	for (const Plugin* p : plugins)
	{
		model::add(pluginManager::makePlugin(*p));
		out.push_back(&model::back<Plugin>());
	}
	return out;
}

/* -------------------------------------------------------------------------- */

void setPluginParameter(ID pluginId, int paramIndex, float value)
{
	model::find<Plugin>(pluginId)->setParameter(paramIndex, value);
}

/* -------------------------------------------------------------------------- */

void setPluginProgram(ID pluginId, int programIndex)
{
	model::find<Plugin>(pluginId)->setCurrentProgram(programIndex);
}

/* -------------------------------------------------------------------------- */

void toggleBypass(ID pluginId)
{
	Plugin& plugin = *model::find<Plugin>(pluginId);
	plugin.setBypass(!plugin.isBypassed());
}

/* -------------------------------------------------------------------------- */

void runDispatchLoop()
{
	messageManager_->runDispatchLoopUntil(10);
}
} // namespace giada::m::pluginHost

#endif // #ifdef WITH_VST
