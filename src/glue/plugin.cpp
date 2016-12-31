/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * glue
 * Intermediate layer GUI <-> CORE.
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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


#include "../core/pluginHost.h"
#include "../core/mixer.h"
#include "../core/plugin.h"
#include "../core/channel.h"
#include "plugin.h"


extern Mixer	    G_Mixer;
extern PluginHost G_PluginHost;


Plugin *glue_addPlugin(Channel *ch, int index, int stackType)
{
  if (index >= G_PluginHost.countAvailablePlugins())
    return nullptr;

  return G_PluginHost.addPlugin(index, stackType,  &G_Mixer.mutex_plugins, ch);
}


/* -------------------------------------------------------------------------- */


void glue_swapPlugins(Channel *ch, int index1, int index2, int stackType)
{
  G_PluginHost.swapPlugin(index1, index2, stackType, &G_Mixer.mutex_plugins,
    ch);
}


/* -------------------------------------------------------------------------- */


void glue_freePlugin(Channel *ch, int index, int stackType)
{
  G_PluginHost.freePlugin(index, stackType, &G_Mixer.mutex_plugins, ch);
}


#endif
