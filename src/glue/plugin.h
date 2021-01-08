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


#ifndef G_GLUE_PLUGIN_H
#define G_GLUE_PLUGIN_H


#ifdef WITH_VST


#include <vector>
#include <string>
#include "core/plugins/pluginHost.h"
#include "core/types.h"


namespace juce { class AudioProcessorEditor; }
namespace giada::m { class Plugin; }
namespace giada::m::channel { struct Data; }
namespace giada::c::plugin 
{
struct Program
{
    int         index;
    std::string name;
};

struct Param
{
    Param() = default;
    Param(const m::Plugin&, int index, ID channelId);

    int         index;
    ID          pluginId;
    ID          channelId;
    std::string name;
    std::string text;
    std::string label;
    float       value;
};

struct Plugin
{
    Plugin(m::Plugin&, ID channelId);

    juce::AudioProcessorEditor* createEditor() const;
    const m::Plugin& getPluginRef() const;

    void setResizeCallback(std::function<void(int, int)> f);

    ID          id;
    ID          channelId;
    bool        valid;
    bool        hasEditor;
    bool        isBypassed;
    std::string name;
    std::string uniqueId;
    int         currentProgram;

    std::vector<Program> programs;
    std::vector<int>     paramIndexes;

private:

    m::Plugin& m_plugin;
};

struct Plugins
{
    Plugins() = default;
    Plugins(const m::channel::Data&);

    ID channelId;
    std::vector<m::Plugin*> plugins; 
};

/* get*
Returns ViewModel objects. */

Plugins getPlugins(ID channelId);
Plugin  getPlugin (m::Plugin& plugin, ID channelId);
Param   getParam  (int index, const m::Plugin& plugin, ID channelId);

/* updateWindow
Updates the editor-less plug-in window. This is useless if the plug-in has an
editor. */

void updateWindow(ID pluginId, bool gui);

void addPlugin(int pluginListIndex, ID channelId);
void swapPlugins(const m::Plugin& p1, const m::Plugin& p2, ID channelId);
void freePlugin(const m::Plugin& plugin, ID channelId);
void setProgram(ID pluginId, int programIndex);
void toggleBypass(ID pluginId);

/* setPluginPathCb
Callback attached to the DirBrowser for adding new Plug-in search paths in the
configuration window. */

void setPluginPathCb(void* data);
} // giada::c::plugin::


#endif


#endif
