/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


using namespace giada::m;


Plugin *glue_addPlugin(Channel *ch, int index, int stackType)
{
  if (index >= pluginHost::countAvailablePlugins())
    return nullptr;

  return pluginHost::addPlugin(index, stackType, &mixer::mutex_plugins, ch);
}


/* -------------------------------------------------------------------------- */


void glue_swapPlugins(Channel *ch, int index1, int index2, int stackType)
{
  pluginHost::swapPlugin(index1, index2, stackType, &mixer::mutex_plugins,
    ch);
}


/* -------------------------------------------------------------------------- */


void glue_freePlugin(Channel *ch, int index, int stackType)
{
  pluginHost::freePlugin(index, stackType, &mixer::mutex_plugins, ch);
}


#endif
