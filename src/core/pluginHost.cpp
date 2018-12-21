/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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


using std::vector;
using std::string;


namespace giada {
namespace m {
namespace pluginHost
{
namespace
{
juce::MessageManager* messageManager_;
juce::AudioBuffer<float> audioBuffer_;

vector<Plugin*> masterOut_;
vector<Plugin*> masterIn_;


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


void addPlugin(Plugin* p, int stackType, pthread_mutex_t* mixerMutex, Channel* ch)
{
	vector<Plugin*>* stack = getStack(stackType, ch);

	pthread_mutex_lock(mixerMutex);
	stack->push_back(p);
	pthread_mutex_unlock(mixerMutex);

	gu_log("[pluginHost::addPlugin] plugin loaded (%s), stack type=%d, stack size=%d\n",
		p->getName().c_str(), stackType, stack->size());
}


/* -------------------------------------------------------------------------- */


vector<Plugin*>* getStack(int stackType, Channel* ch)
{
	switch(stackType) {
		case MASTER_OUT:
			return &masterOut_;
		case MASTER_IN:
			return &masterIn_;
		case CHANNEL:
			return &ch->plugins;
		default:
			return nullptr;
	}
}


/* -------------------------------------------------------------------------- */


int countPlugins(int stackType, Channel* ch)
{
	return getStack(stackType, ch)->size();
}


/* -------------------------------------------------------------------------- */


void freeStack(int stackType, pthread_mutex_t* mixerMutex, Channel* ch)
{
	vector<Plugin*>* pStack = getStack(stackType, ch);

	if (pStack->size() == 0)
		return;

	pthread_mutex_lock(mixerMutex);
		for (Plugin* p : *pStack)
			delete p;
		pStack->clear();
	pthread_mutex_unlock(mixerMutex);

	gu_log("[pluginHost::freeStack] stack type=%d freed\n", stackType);
}


/* -------------------------------------------------------------------------- */


void processStack(AudioBuffer& outBuf, int stackType, Channel* ch)
{
	vector<Plugin*>* stack = getStack(stackType, ch);

	/* Empty stack, stack not found or mixer not ready: do nothing. */

	if (stack == nullptr || stack->size() == 0)
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

	for (Plugin* plugin : *stack)
		processPlugin_(*plugin, ch);

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


Plugin* getPluginByIndex(int index, int stackType, Channel* ch)
{
	vector<Plugin*>* pStack = getStack(stackType, ch);
	assert((size_t) index < pStack->size());
	return pStack->at(index);
}


/* -------------------------------------------------------------------------- */


int getPluginIndex(int id, int stackType, Channel* ch)
{
	vector<Plugin*>* stack = getStack(stackType, ch);
	return u::vector::indexOf(*stack, [&](const Plugin* p) { return p->getId() == id; });
}


/* -------------------------------------------------------------------------- */


void swapPlugin(unsigned indexA, unsigned indexB, int stackType,
	pthread_mutex_t* mixerMutex, Channel* ch)
{
	vector<Plugin*>* stack = getStack(stackType, ch);

	pthread_mutex_lock(mixerMutex);
	std::swap(stack->at(indexA), stack->at(indexB));
	pthread_mutex_unlock(mixerMutex);

	gu_log("[pluginHost::swapPlugin] plugin at index %d and %d swapped\n", indexA, indexB);
}


/* -------------------------------------------------------------------------- */


int freePlugin(int id, int stackType, pthread_mutex_t* mixerMutex, Channel* ch)
{
	vector<Plugin*>* stack = getStack(stackType, ch);

	int index = u::vector::indexOf(*stack, [&](const Plugin* p) { return p->getId() == id; });
	assert(index != -1);

	pthread_mutex_lock(mixerMutex);
	delete stack->at(index);
	stack->erase(stack->begin() + index);
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


void freeAllStacks(vector<Channel*>* channels, pthread_mutex_t* mixerMutex)
{
	freeStack(pluginHost::MASTER_OUT, mixerMutex);
	freeStack(pluginHost::MASTER_IN, mixerMutex);
	for (Channel* c : *channels)
		freeStack(pluginHost::CHANNEL, mixerMutex, c);
}


/* -------------------------------------------------------------------------- */


int clonePlugin(Plugin* src, int stackType, pthread_mutex_t* mixerMutex,
	Channel* ch)
{
	assert(false);
#if 0

TOD0

	Plugin* p = addPlugin(src->getUniqueId(), stackType, mutex, ch);
	if (!p) {
		gu_log("[pluginHost::clonePlugin] unable to add new plugin to stack!\n");
		return 0;
	}

	for (int k=0; k<src->getNumParameters(); k++)
		p->setParameter(k, src->getParameter(k));
#endif
	return 1;
}


/* -------------------------------------------------------------------------- */


void forEachPlugin(int stackType, const Channel* ch, std::function<void(const Plugin* p)> f)
{
	/* TODO - Remove const is ugly. This is a temporary workaround until all
	PluginHost functions params will be const-correct. */
	vector<Plugin*>* stack = getStack(stackType, const_cast<Channel*>(ch));
	for (const Plugin* p : *stack)
		f(p);
}


}}}; // giada::m::pluginHost::


#endif // #ifdef WITH_VST
