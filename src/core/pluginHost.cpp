/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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

#include <cassert>
#include "utils/log.h"
#include "utils/vector.h"
#include "core/model/model.h"
#include "core/channels/channel.h"
#include "core/const.h"
#include "core/plugin.h"
#include "core/pluginManager.h"
#include "core/pluginHost.h"


namespace giada {
namespace m {
namespace pluginHost
{
namespace
{
juce::MessageManager* messageManager_;
juce::AudioBuffer<float> audioBuffer_;
ID pluginId_;


/* -------------------------------------------------------------------------- */


void giadaToJuceTempBuf_(const AudioBuffer& outBuf)
{
	for (int i=0; i<outBuf.countFrames(); i++)
		for (int j=0; j<outBuf.countChannels(); j++)
			audioBuffer_.setSample(j, i, outBuf[i][j]);
}


/* juceToGiadaOutBuf_
Converts buffer from Juce to Giada. A note for the future: if we overwrite (=) 
(as we do now) it's SEND, if we add (+) it's INSERT. */

void juceToGiadaOutBuf_(AudioBuffer& outBuf)
{
	for (int i=0; i<outBuf.countFrames(); i++)
		for (int j=0; j<outBuf.countChannels(); j++)	
			outBuf[i][j] = audioBuffer_.getSample(j, i);
}


/* -------------------------------------------------------------------------- */


void processPlugins_(const std::vector<ID>& pluginIds, juce::MidiBuffer& events)
{
	model::PluginsLock l(model::plugins);

	for (ID id : pluginIds) {
		Plugin& p = model::get(model::plugins, id);
		if (!p.valid || p.isSuspended() || p.isBypassed())
			continue;
		p.process(audioBuffer_, events);
		events.clear();
	}
}


ID clonePlugin_(ID pluginId)
{
	model::PluginsLock l(model::plugins);

	const Plugin&           original = model::get(model::plugins, pluginId);
	std::unique_ptr<Plugin> clone    = pluginManager::makePlugin(original);
	ID                      newId    = clone->id;

	model::plugins.push(std::move(clone));

	return newId;
}
}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void close()
{
	messageManager_->deleteInstance();
	model::plugins.clear();
}


/* -------------------------------------------------------------------------- */


void init(int buffersize)
{
	messageManager_ = juce::MessageManager::getInstance();
	audioBuffer_.setSize(G_MAX_IO_CHANS, buffersize);
	pluginId_ = 0;
}


/* -------------------------------------------------------------------------- */


void processStack(AudioBuffer& outBuf, const std::vector<ID>& pluginIds, 
	juce::MidiBuffer* events)
{
	assert(outBuf.countFrames() == audioBuffer_.getNumSamples());

	/* If events are null: Audio stack processing (master in, master out or
	sample channels. No need for MIDI events. 
	If events are not null: MIDI stack (MIDI channels). MIDI channels must not 
	process the current buffer: give them an empty and clean one. */
	
	if (events == nullptr) {
		giadaToJuceTempBuf_(outBuf);
		juce::MidiBuffer events; // empty
		processPlugins_(pluginIds, events);
	}
	else {
		audioBuffer_.clear();
		processPlugins_(pluginIds, *events);

	}
	juceToGiadaOutBuf_(outBuf);
}


/* -------------------------------------------------------------------------- */


void addPlugin(std::unique_ptr<Plugin> p, ID channelId)
{
	ID pluginId = p->id;
	
	model::plugins.push(std::move(p));

	model::onSwap(model::channels, channelId, [&](Channel& c)
	{
		c.pluginIds.push_back(pluginId);
	});
}


/* -------------------------------------------------------------------------- */


void swapPlugin(ID pluginId1, ID pluginId2, ID channelId)
{
	model::onSwap(model::channels, channelId, [&](Channel& c)
	{
		auto a = u::vector::indexOf(c.pluginIds, pluginId1); 
		auto b = u::vector::indexOf(c.pluginIds, pluginId2); 
	
		std::swap(c.pluginIds.at(a), c.pluginIds.at(b));
	});
}


/* -------------------------------------------------------------------------- */


void freePlugin(ID pluginId, ID channelId)
{
	model::onSwap(model::channels, channelId, [&](Channel& c)
	{
		u::vector::remove(c.pluginIds, pluginId);
	});

	model::plugins.pop(model::getIndex(model::plugins, pluginId));
}


void freePlugins(const std::vector<ID>& pluginIds)
{
	for (ID id : pluginIds)
		model::plugins.pop(model::getIndex(model::plugins, id));
}


/* -------------------------------------------------------------------------- */


void clonePlugins(const Channel& oldChannel, Channel& newChannel)
{
	newChannel.pluginIds.clear();
	for (ID id : oldChannel.pluginIds)
		newChannel.pluginIds.push_back(clonePlugin_(id));
}


/* -------------------------------------------------------------------------- */


void setPluginParameter(ID pluginId, int paramIndex, float value)
{
	model::onGet(model::plugins, pluginId, [&](Plugin& p)
	{
		p.setParameter(paramIndex, value);
	});
}


/* -------------------------------------------------------------------------- */


void setPluginProgram(ID pluginId, int programIndex)
{
	model::onGet(model::plugins, pluginId, [&](Plugin& p)
	{
		p.setCurrentProgram(programIndex);
	});
}


/* -------------------------------------------------------------------------- */


void toggleBypass(ID pluginId)
{
	model::onGet(model::plugins, pluginId, [&](Plugin& p)
	{
		p.setBypass(!p.isBypassed());
	});
}


/* -------------------------------------------------------------------------- */


void runDispatchLoop()
{
	messageManager_->runDispatchLoopUntil(10);
}

}}}; // giada::m::pluginHost::


#endif // #ifdef WITH_VST
