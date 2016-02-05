/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * pluginHost
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


#include "../utils/log.h"
#include "../utils/utils.h"
#include "pluginHost.h"


using std::string;


PluginHost::PluginHost()
{
  /* TODO - use real paths! */

  #if defined(__linux__)
    pluginDirs = "/usr/local/lib/vst/linux";
  #elif defined(_WIN32)
    pluginDirs = "c:\\Users\\mcl\\giada\\vst";
  #else
    pluginDirs = "/home/mcl/.vst";
  #endif
}


/* -------------------------------------------------------------------------- */


int PluginHost::scanDir()
{
  gLog("[PluginHost::scanDir] plugin directory = '%s'\n", pluginDirs.c_str());

  juce::VSTPluginFormat format;
  juce::FileSearchPath path(pluginDirs);
  juce::PluginDirectoryScanner scanner(knownPluginList, format, path, false, juce::File::nonexistent);
  juce::String name;

  bool cont = true;
  while (cont) {
    cont = scanner.scanNextFile(false, name);
  }

  gLog("[PluginHost::scanDir] %d plugin(s) found\n", knownPluginList.getNumTypes());

  return knownPluginList.getNumTypes();
}


/* -------------------------------------------------------------------------- */


int PluginHost::saveList(const string &filepath)
{
  knownPluginList.createXml()->writeToFile(juce::File(filepath), "");
  return gFileExists(filepath.c_str());
}


/* -------------------------------------------------------------------------- */


Plugin *PluginHost::addPlugin(int index, int stackType, class Channel *ch)
{
  //pluginFormat.createInstanceFromDescription(*pl.getType(PLUGIN_TO_OPEN), 44100, 1024);
  return NULL;
}

#endif // #ifdef WITH_VST
