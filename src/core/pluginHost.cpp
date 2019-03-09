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
#include "core/model/data.h"
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


void processPlugins_(const Stack& stack, juce::MidiBuffer& events)
{
	for (const std::shared_ptr<Plugin>& p : stack) {
		if (p->isSuspended() || p->isBypassed())
			continue;
		const_cast<std::shared_ptr<Plugin>&>(p)->process(audioBuffer_, events);
		events.clear();
	}
}
}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void close()
{
	messageManager_->deleteInstance();
}


/* -------------------------------------------------------------------------- */


void init(int buffersize)
{
	messageManager_ = juce::MessageManager::getInstance();
	audioBuffer_.setSize(G_MAX_IO_CHANS, buffersize);
	pluginId_ = 0;
}


/* -------------------------------------------------------------------------- */


void processStack(AudioBuffer& outBuf, const Stack& stack, juce::MidiBuffer* events)
{
	assert(outBuf.countFrames() == audioBuffer_.getNumSamples());

	/* If events are null: Audio stack processing (master in, master out or
	sample channels. No need for MIDI events. 
	If events are not null: MIDI stack (MIDI channels). MIDI channels must not 
	process the current buffer: give them an empty and clean one. */
	
	if (events == nullptr) {
		giadaToJuceTempBuf_(outBuf);
		juce::MidiBuffer events; // empty
		processPlugins_(stack, events);
	}
	else {
		audioBuffer_.clear();
		processPlugins_(stack, *events);

	}
	juceToGiadaOutBuf_(outBuf);
}


/* -------------------------------------------------------------------------- */


const Plugin* getPluginByID(ID pluginID, ID chanID)
{
	return model::getLayout()->getPlugin(pluginID, chanID);
}


/* -------------------------------------------------------------------------- */


void addPlugin(std::shared_ptr<Plugin> p, ID chanID)
{
	Stack& stack = model::getData().plugins[chanID]; // Let's create the key if non-existent with the [] operator
	p->id = pluginId_++;
	stack.push_back(p);
	
	std::shared_ptr<model::Layout> layout = model::cloneLayout();
	layout->getPlugins(chanID)->push_back(p);
	model::swapLayout(layout);
}


/* -------------------------------------------------------------------------- */


void swapPlugin(ID pluginID1, ID pluginID2, ID chanID)
{
	if (model::getLayout()->getPlugins(chanID)->size() == 1 || pluginID1 == pluginID2)
		return;

	std::shared_ptr<model::Layout> layout = model::cloneLayout();
	Stack& stack = *layout->getPlugins(chanID);

	int index1 = u::vector::indexOf(stack, [=](const std::shared_ptr<Plugin>& p)
	{
		return p->id == pluginID1;
	});
	int index2 = u::vector::indexOf(stack, [=](const std::shared_ptr<Plugin>& p)
	{
		return p->id == pluginID2;
	});

	std::swap(stack[index1], stack[index2]);
	model::swapLayout(layout);
}


/* -------------------------------------------------------------------------- */


void freePlugin(ID pluginID, ID chanID)
{
	std::shared_ptr<model::Layout> layout = model::cloneLayout();
	u::vector::removeIf(*layout->getPlugins(chanID), [=](const std::shared_ptr<Plugin>& p)
	{
		return p->id == pluginID;
	});
	model::swapLayout(layout);

	u::vector::removeIf(model::getData().plugins.at(chanID), [=](const std::shared_ptr<Plugin>& p)
	{
		return p->id == pluginID;
	});
}


/* -------------------------------------------------------------------------- */


void clonePlugins(ID channelId, ID newChannelId)
{
	Stack& stack = model::getData().plugins[newChannelId]; // Create the key
	
	for (std::shared_ptr<Plugin>& p : *model::getLayout()->getPlugins(channelId)) {
		std::shared_ptr<Plugin> clone = pluginManager::makePlugin(*p.get()); 
		clone->id = pluginId_++;
		stack.push_back(clone);
	}
}


/* -------------------------------------------------------------------------- */


void setPluginParameter(ID pluginID, int paramIndex, float value, ID chanID)
{
	model::getLayout()->getPlugin(pluginID, chanID)->setParameter(paramIndex, value);
}


/* -------------------------------------------------------------------------- */


void setPluginProgram(ID pluginID, int programIndex, ID chanID)
{
	model::getLayout()->getPlugin(pluginID, chanID)->setCurrentProgram(programIndex);
}


/* -------------------------------------------------------------------------- */


void toggleBypass(ID pluginID, ID chanID)
{
	Plugin* p = model::getLayout()->getPlugin(pluginID, chanID);
	p->setBypass(!p->isBypassed());
}


/* -------------------------------------------------------------------------- */


void runDispatchLoop()
{
	messageManager_->runDispatchLoopUntil(10);
}

}}}; // giada::m::pluginHost::


#endif // #ifdef WITH_VST
