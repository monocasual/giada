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
#include "deps/juce-config.h"
#include "core/types.h"


namespace giada {
namespace m 
{
class Plugin;
class Channel;
class AudioBuffer;

namespace pluginHost
{
using Stack = std::vector<std::shared_ptr<Plugin>>;

void init(int buffersize);
void close();

/* addPlugin
Adds a new plugin to channel 'channelId'. */

void addPlugin(std::shared_ptr<Plugin> p, ID channelId);

/* processStack
Applies the fx list to the buffer. */

void processStack(AudioBuffer& outBuf, const Stack& s, juce::MidiBuffer* events=nullptr);

/* getPluginByIndex */

const Plugin* getPluginByID(ID pluginId, ID channelId);

/* swapPlugin 
Swaps plug-in at index1 with plug-in at index 2 in Channel 'channelId'. */

void swapPlugin(ID pluginId1, ID pluginId2, ID channelId);

/* freePlugin.
Unloads plugin 'pluginId' from channel 'channelId'. */

void freePlugin(ID pluginId, ID channelId);

/* clonePlugins
Clones all the plug-ins from the current channel to the new one. */

void clonePlugins(ID channelId, ID newChannelId);

void setPluginParameter(ID pluginId, int paramIndex, float value, ID channelId);
void setPluginProgram(ID pluginId, int programIndex, ID channelId); 

void toggleBypass(ID pluginId, ID channelId);

/* runDispatchLoop
Wakes up plugins' GUI manager for N milliseconds. */

void runDispatchLoop();
}}}; // giada::m::pluginHost::


#endif

#endif // #ifdef WITH_VST
