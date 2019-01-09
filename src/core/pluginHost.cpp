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
#include "../utils/log.h"
#include "../utils/vector.h"
#include "const.h"
#include "channel.h"
#include "plugin.h"
#include "pluginHost.h"


namespace giada {
namespace m {
namespace pluginHost
{
namespace
{
juce::MessageManager* messageManager_;
juce::AudioBuffer<float> audioBuffer_;

std::vector<std::unique_ptr<Plugin>> masterOut_;
std::vector<std::unique_ptr<Plugin>> masterIn_;


/* -------------------------------------------------------------------------- */


void processPlugin_(Plugin& p, Channel* ch)
{
	if (p.isSuspended() || p.isBypassed())
		return;

	juce::MidiBuffer events;
	if (ch != nullptr)
		events = ch->getPluginMidiEvents();

	p.process(audioBuffer_, events);
}


/* -------------------------------------------------------------------------- */

/* getStack_
Returns a vector of unique_ptr's given the stackType. If stackType == CHANNEL
a pointer to Channel is also required. */

std::vector<std::unique_ptr<Plugin>>& getStack_(StackType t, Channel* ch=nullptr)
{
	switch(t) {
		case StackType::MASTER_OUT:
			return masterOut_;
		case StackType::MASTER_IN:
			return masterIn_;
		case StackType::CHANNEL:
			return ch->plugins;
		default:
			assert(false);
	}
}

}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


pthread_mutex_t mutex;


/* -------------------------------------------------------------------------- */


void close()
{
	messageManager_->deleteInstance();
	pthread_mutex_destroy(&mutex);
}


/* -------------------------------------------------------------------------- */


void init(int buffersize)
{
	messageManager_ = juce::MessageManager::getInstance();
	audioBuffer_.setSize(G_MAX_IO_CHANS, buffersize);

	pthread_mutex_init(&mutex, nullptr);
}


/* -------------------------------------------------------------------------- */


void addPlugin(std::unique_ptr<Plugin> p, StackType t, pthread_mutex_t* mixerMutex, 
	Channel* ch)
{
	std::vector<std::unique_ptr<Plugin>>& stack = getStack_(t, ch);

	gu_log("[pluginHost::addPlugin] load plugin (%s), stack type=%d, stack size=%d\n",
		p->getName().c_str(), t, stack.size());

	pthread_mutex_lock(mixerMutex);
	stack.push_back(std::move(p));
	pthread_mutex_unlock(mixerMutex);
}


/* -------------------------------------------------------------------------- */


std::vector<Plugin*> getStack(StackType t, Channel* ch)
{
	std::vector<std::unique_ptr<Plugin>>& stack = getStack_(t, ch);

	std::vector<Plugin*> out;
	for (const std::unique_ptr<Plugin>& p : stack)
		out.push_back(p.get());

	return out;
}


/* -------------------------------------------------------------------------- */


int countPlugins(StackType t, Channel* ch)
{
	return getStack_(t, ch).size();
}


/* -------------------------------------------------------------------------- */


void freeStack(StackType t, pthread_mutex_t* mixerMutex, Channel* ch)
{
	std::vector<std::unique_ptr<Plugin>>& stack = getStack_(t, ch);

	if (stack.size() == 0)
		return;

	pthread_mutex_lock(mixerMutex);
	stack.clear();
	pthread_mutex_unlock(mixerMutex);

	gu_log("[pluginHost::freeStack] stack type=%d freed\n", t);
}


/* -------------------------------------------------------------------------- */


void processStack(AudioBuffer& outBuf, StackType t, Channel* ch)
{
	std::vector<std::unique_ptr<Plugin>>& stack = getStack_(t, ch);

	if (stack.size() == 0)
		return;

	assert(outBuf.countFrames() == audioBuffer_.getNumSamples());

	/* MIDI channels must not process the current buffer: give them an empty one. 
	Sample channels and Master in/out want audio data instead: let's convert the 
	internal buffer from Giada to Juce. */

	if (ch != nullptr && ch->type == ChannelType::MIDI) 
		audioBuffer_.clear();
	else
		for (int i=0; i<outBuf.countFrames(); i++)
			for (int j=0; j<outBuf.countChannels(); j++)
				audioBuffer_.setSample(j, i, outBuf[i][j]);

	/* Hardcore processing. Part of this loop must be guarded by mutexes, i.e. 
	the MIDI process part. You definitely don't want a situation like the 
	following one:
		1. this::processStack()
		2. [a new midi event comes in from kernelMidi thread]
		3. channel::clearMidiBuffer()
	The midi event in between would be surely lost, deleted by the last call to
	channel::clearMidiBuffer()! 
	TODO - that's why we need a proper queue for MIDI events in input... */

	if (ch != nullptr)
		pthread_mutex_lock(&mutex);

	for (std::unique_ptr<Plugin>& plugin : stack)
		processPlugin_(*plugin.get(), ch);

	if (ch != nullptr) {
		ch->clearMidiBuffer();
		pthread_mutex_unlock(&mutex);
	}

	/* Converting buffer from Juce to Giada. A note for the future: if we 
	overwrite (=) (as we do now) it's SEND, if we add (+) it's INSERT. */

	for (int i=0; i<outBuf.countFrames(); i++)
		for (int j=0; j<outBuf.countChannels(); j++)	
			outBuf[i][j] = audioBuffer_.getSample(j, i);
}


/* -------------------------------------------------------------------------- */


Plugin* getPluginByIndex(int index, StackType t, Channel* ch)
{
	std::vector<std::unique_ptr<Plugin>>& stack = getStack_(t, ch);
	assert((size_t) index < stack.size());
	return stack.at(index).get();
}


/* -------------------------------------------------------------------------- */


int getPluginIndex(int id, StackType t, Channel* ch)
{
	std::vector<std::unique_ptr<Plugin>>& stack = getStack_(t, ch);
	return u::vector::indexOf(stack, [&](const std::unique_ptr<Plugin>& p) 
	{ 
		return p->getId() == id;
	});
}


/* -------------------------------------------------------------------------- */


void swapPlugin(int indexA, int indexB, StackType t, pthread_mutex_t* mixerMutex, 
	Channel* ch)
{
	std::vector<std::unique_ptr<Plugin>>& stack = getStack_(t, ch);

	pthread_mutex_lock(mixerMutex);
	std::swap(stack.at(indexA), stack.at(indexB));
	pthread_mutex_unlock(mixerMutex);

	gu_log("[pluginHost::swapPlugin] plugin at index %d and %d swapped\n", indexA, indexB);
}


/* -------------------------------------------------------------------------- */


int freePlugin(int id, StackType t, pthread_mutex_t* mixerMutex, Channel* ch)
{
	std::vector<std::unique_ptr<Plugin>>& stack = getStack_(t, ch);

	int index = u::vector::indexOf(stack, [&](const std::unique_ptr<Plugin>& p) 
	{ 
		return p->getId() == id; 
	});
	assert(index != -1);

	pthread_mutex_lock(mixerMutex);
	stack.erase(stack.begin() + index);
	pthread_mutex_unlock(mixerMutex);	

	gu_log("[pluginHost::freePlugin] plugin id=%d removed\n", id);
	return index;
}


/* -------------------------------------------------------------------------- */


void runDispatchLoop()
{
	messageManager_->runDispatchLoopUntil(10);
}


/* -------------------------------------------------------------------------- */


void freeAllStacks(std::vector<Channel*>* channels, pthread_mutex_t* mixerMutex)
{
	freeStack(StackType::MASTER_OUT, mixerMutex);
	freeStack(StackType::MASTER_IN, mixerMutex);
	for (Channel* c : *channels)
		freeStack(StackType::CHANNEL, mixerMutex, c);
}


/* -------------------------------------------------------------------------- */


void forEachPlugin(StackType t, const Channel* ch, std::function<void(const Plugin* p)> f)
{
	std::vector<std::unique_ptr<Plugin>>& stack = getStack_(t, const_cast<Channel*>(ch));
	for (const std::unique_ptr<Plugin>& p : stack)
		f(p.get());
}

}}}; // giada::m::pluginHost::


#endif // #ifdef WITH_VST
