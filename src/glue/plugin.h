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


#ifndef G_GLUE_PLUGIN_H
#define G_GLUE_PLUGIN_H


#ifdef WITH_VST


#include "core/pluginHost.h"
#include "core/types.h"


namespace giada {
namespace m
{
class Plugin;
class Channel;
}
namespace c {
namespace plugin 
{
void addPlugin(int pluginListIndex, ID chanID);

void swapPlugins(ID pluginID1, ID pluginID2, ID chanID);

void freePlugin(ID pluginID, ID chanID);

void setParameter(ID pluginID, int paramIndex, float value, ID chanID, 
    bool gui=true); 

void setProgram(ID pluginID, int programIndex, ID chanID);

void toggleBypass(ID pluginID, ID chanID);

/* setPluginPathCb
Callback attached to the DirBrowser for adding new Plug-in search paths in the
configuration window. */

void setPluginPathCb(void* data);
}}}; // giada::c::plugin::


#endif


#endif
