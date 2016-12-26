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

  Plugin *plugin = G_PluginHost.addPlugin(index, stackType,
    &G_Mixer.mutex_plugins, ch);

  /* Fill Channel::midiInPlugins vector for Midi learning. */

  vector<uint32_t> midiInParams;
  for (int i=0; i<plugin->getNumParameters(); i++)
    midiInParams.push_back(0x0);
  ch->midiInPlugins.push_back(midiInParams);

  return plugin;
}


/* -------------------------------------------------------------------------- */


void glue_swapPlugins(Channel *ch, int index1, int index2, int stackType)
{
  G_PluginHost.swapPlugin(index1, index2, stackType, &G_Mixer.mutex_plugins,
    ch);

  /* Swap items in Channel::midiInPlugins vector. */

  std::swap(ch->midiInPlugins.at(index1), ch->midiInPlugins.at(index2));
}


/* -------------------------------------------------------------------------- */


void glue_freePlugin(Channel *ch, int index, int stackType)
{
  int pi = G_PluginHost.freePlugin(index, stackType, &G_Mixer.mutex_plugins, ch);

  /* Erase item from Channel::midiInPlugins vector. */

  if (pi == -1) // something went wrong with deletion
    return;
  ch->midiInPlugins.erase(ch->midiInPlugins.begin() + pi);
}


#endif
