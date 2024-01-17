/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_PLUGIN_MANAGER_H
#define G_PLUGIN_MANAGER_H

#include "core/patch.h"
#include "plugin.h"
#include <memory>

namespace giada::m::patch
{
struct Plugin;
struct Version;
} // namespace giada::m::patch

namespace giada::m::model
{
class Model;
class Sequencer;
} // namespace giada::m::model

namespace giada::m
{
class PluginManager final
{
public:
	enum class SortMethod : int
	{
		NAME = 0,
		CATEGORY,
		MANUFACTURER,
		FORMAT
	};

	enum class SortDir : int
	{
		ASC = 0,
		DESC
	};

	struct SortMode
	{
		SortMethod method;
		SortDir    dir;
	};

	struct PluginInfo
	{
		std::string uid;
		std::string name;
		std::string category;
		std::string manufacturerName;
		std::string format;
		bool        isInstrument;
		bool        exists;
		bool        isKnown;
	};

	/* getPluginsInfo
	Returns a vector of PluginInfo objects containing all plug-ins, known and
	unknown, scanned so far. */

	std::vector<PluginInfo> getPluginsInfo() const;

	/* hasMissingPlugins
	True if some plug-ins have been marked as missing during the initial scan. */

	bool hasMissingPlugins() const;

	/* countAvailablePlugins
	Returns how many plug-ins are ready and available for usage. */

	int countAvailablePlugins() const;

	/* reset
	Brings everything back to the initial state. */

	void reset();

	/* scanDirs
	Parses plugin directories (semicolon-separated) and store list in 
	knownPluginList. The callback is called on each plugin found. Used to update
	the main window from the GUI thread. Return false from the progress callback 
	to stop	the scanning process. */

	int scanDirs(const std::string& paths, std::function<bool(float)> progressCb);

	/* (save|load)List
	(Save|Load) knownPluginList (in|from) an XML file. */

	bool saveList(const std::string& path) const;
	bool loadList(const std::string& path);

	std::unique_ptr<Plugin> makePlugin(const std::string& pid, int sampleRate, int bufferSize, const model::Sequencer&, ID id = 0);
	std::unique_ptr<Plugin> makePlugin(int index, int sampleRate, int bufferSize, const model::Sequencer&);
	std::unique_ptr<Plugin> makePlugin(const Plugin& other, int sampleRate, int bufferSize, const model::Sequencer&);

	std::unique_ptr<juce::AudioPluginInstance> makeJucePlugin(const std::string& pid, int sampleRate, int bufferSize);

	/* clonePlugins
	Clones all plugins in the Plugin vector passed in as a parameter. Returns a
	new vector containing the new clones. */
	// TODO - move to pluginFactory

	std::vector<Plugin*> clonePlugins(const std::vector<Plugin*>&, int sampleRate, int bufferSize, model::Model&);

	void sortPlugins(SortMode);

private:
	/* formatManager
	Plugin format manager. */

	juce::AudioPluginFormatManager m_formatManager;

	/* knownPuginList
	List of known (i.e. scanned) plugins. */

	juce::KnownPluginList m_knownPluginList;

	/* unknownPluginList
	List of unrecognized plugins found in a patch. */

	std::vector<std::string> m_unknownPluginList;
};
} // namespace giada::m

#endif