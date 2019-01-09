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


#ifndef G_PLUGIN_HOST_H
#define G_PLUGIN_HOST_H


#include <functional>
#include <pthread.h>
#include "../deps/juce-config.h"


namespace giada {
namespace m 
{
class Plugin;
class Channel;
class AudioBuffer;

namespace pluginHost
{
enum class StackType { MASTER_OUT, MASTER_IN, CHANNEL };

extern pthread_mutex_t mutex;

void init(int buffersize);
void close();

/* addPlugin
Adds a new plugin to 'stackType'. */

void addPlugin(std::unique_ptr<Plugin> p, StackType t, pthread_mutex_t* mutex, 
    Channel* ch=nullptr);

/* countPlugins
Returns the size of 'stackType'. */

int countPlugins(StackType t, Channel* ch=nullptr);

/* freeStack
Frees plugin stack of type 'stackType'. */

void freeStack(StackType t, pthread_mutex_t* mutex, Channel* ch=nullptr);

/* processStack
Applies the fx list to the buffer. */

void processStack(AudioBuffer& outBuf, StackType t, Channel* ch=nullptr);

/* getStack
Returns a vector of Plugin pointers given the stackType. If stackType == CHANNEL
a pointer to Channel is also required. */

std::vector<Plugin*> getStack(StackType t, Channel* ch=nullptr);

/* getPluginByIndex */

Plugin* getPluginByIndex(int index, StackType t, Channel* ch=nullptr);

/* getPluginIndex */

int getPluginIndex(int id, StackType t, Channel* ch=nullptr);

/* swapPlugin */

void swapPlugin(int indexA, int indexB, StackType t, pthread_mutex_t* mutex, 
    Channel* ch=nullptr);

/* freePlugin.
Returns the internal stack index of the deleted plugin. */

int freePlugin(int id, StackType t, pthread_mutex_t* mutex, Channel* ch=nullptr);

/* runDispatchLoop
Wakes up plugins' GUI manager for N milliseconds. */

void runDispatchLoop();

/* freeAllStacks
Frees everything. */

void freeAllStacks(std::vector<Channel*>* channels, pthread_mutex_t* mutex);

void forEachPlugin(StackType t, const Channel* ch, std::function<void(const Plugin* p)> f);

}}}; // giada::m::pluginHost::


#endif

#endif // #ifdef WITH_VST
