/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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
#include "deps/juce-config.h"
#include "core/types.h"


namespace giada {
namespace m 
{
class Plugin;
class AudioBuffer;
namespace pluginHost
{
using Stack = std::vector<std::shared_ptr<Plugin>>;

void init(int buffersize);
void close();

/* addPlugin
Adds a new plugin to channel 'channelId'. */

void addPlugin(std::unique_ptr<Plugin> p, ID channelId);

/* processStack
Applies the fx list to the buffer. */

void processStack(AudioBuffer& outBuf, const std::vector<ID>& pluginIds, 
	juce::MidiBuffer* events=nullptr);

/* swapPlugin 
Swaps plug-in with ID 1 with plug-in with ID 2 in Channel 'channelId'. */

void swapPlugin(ID pluginId1, ID pluginId2, ID channelId);

/* freePlugin.
Unloads plugin from channel 'channelId'. */

void freePlugin(ID pluginId, ID channelId);

/* freePlugins
Unloads multiple plugins. Useful when freeing or deleting a channel. */

void freePlugins(const std::vector<ID>& pluginIds);

/* clonePlugins
Clones all the plug-ins from 'pluginIds' vector coming from the old channel
and returns new IDs. */

std::vector<ID> clonePlugins(std::vector<ID> pluginIds);

void setPluginParameter(ID pluginId, int paramIndex, float value);

void setPluginProgram(ID pluginId, int programIndex); 

void toggleBypass(ID pluginId);

/* runDispatchLoop
Wakes up plugins' GUI manager for N milliseconds. */

void runDispatchLoop();
}}}; // giada::m::pluginHost::


#endif

#endif // #ifdef WITH_VST
