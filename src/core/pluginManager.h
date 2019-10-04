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

#ifndef G_PLUGIN_MANAGER_H
#define G_PLUGIN_MANAGER_H


#include "deps/juce-config.h"
#include "plugin.h"


namespace giada {
namespace m
{
namespace patch
{
struct Plugin;
}
namespace pluginManager
{
enum class SortMethod : int
{
	NAME = 0, CATEGORY,	MANUFACTURER, FORMAT
};

struct PluginInfo
{
	std::string uid;
	std::string name;
	std::string category;
	std::string manufacturerName;
	std::string format;
	bool isInstrument;
};

void init(int samplerate, int buffersize);

/* scanDirs
Parses plugin directories (semicolon-separated) and store list in 
knownPluginList. The callback is called on each plugin found. Used to update the 
main window from the GUI thread. */

int scanDirs(const std::string& paths, const std::function<void(float)>& cb);

/* (save|load)List
(Save|Load) knownPluginList (in|from) an XML file. */

int saveList(const std::string& path);
int loadList(const std::string& path);

/* countAvailablePlugins
Returns how many plug-ins are ready and available for usage. */

int countAvailablePlugins();

/* countUnknownPlugins
Returns how many plug-ins are in a unknown/not-found state. */

unsigned countUnknownPlugins();

std::unique_ptr<Plugin> makePlugin(const std::string& fid);
std::unique_ptr<Plugin> makePlugin(int index);
std::unique_ptr<Plugin> makePlugin(const Plugin& other);
std::unique_ptr<Plugin> makePlugin(const patch::Plugin& p);

/* getAvailablePluginInfo
Returns the available plugin information (name, type, ...) given a plug-in
index. */

PluginInfo getAvailablePluginInfo(int index);

std::string getUnknownPluginInfo(int index);

bool doesPluginExist(const std::string& fid);

bool hasMissingPlugins();

void sortPlugins(SortMethod sortMethod);

}}}; // giada::m::pluginManager::


#endif

#endif // #ifdef WITH_VST
