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

#ifndef G_PLUGIN_HOST_H
#define G_PLUGIN_HOST_H

#include "core/types.h"
#include "deps/juce-config.h"
#include <functional>

namespace mcl
{
class AudioBuffer;
}
namespace giada::m
{
class Plugin;
} // namespace giada::m
namespace giada::m::pluginHost
{
struct Info : public juce::AudioPlayHead
{
	bool getCurrentPosition(CurrentPositionInfo& result) override;
	bool canControlTransport() override;
};

/* -------------------------------------------------------------------------- */

void init(int buffersize);
void close();

/* addPlugin
Adds a new plugin to channel 'channelId'. */

void addPlugin(std::unique_ptr<Plugin> p, ID channelId);

/* processStack
Applies the fx list to the buffer. */

void processStack(mcl::AudioBuffer& outBuf, const std::vector<Plugin*>& plugins,
    juce::MidiBuffer* events = nullptr);

/* swapPlugin 
Swaps plug-in 1 with plug-in 2 in Channel 'channelId'. */

void swapPlugin(const m::Plugin& p1, const m::Plugin& p2, ID channelId);

/* freePlugin.
Unloads plugin from channel 'channelId'. */

void freePlugin(const m::Plugin& plugin, ID channelId);

/* freePlugins
Unloads multiple plugins. Useful when freeing or deleting a channel. */

void freePlugins(const std::vector<Plugin*>& plugins);

/* clonePlugins
Clones all the plug-ins in the 'plugins' vector. */

std::vector<Plugin*> clonePlugins(const std::vector<Plugin*>& plugins);

void setPluginParameter(ID pluginId, int paramIndex, float value);
void setPluginProgram(ID pluginId, int programIndex);
void toggleBypass(ID pluginId);

/* runDispatchLoop
Wakes up plugins' GUI manager for N milliseconds. */

void runDispatchLoop();
} // namespace giada::m::pluginHost

#endif

#endif // #ifdef WITH_VST
