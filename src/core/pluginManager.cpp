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


#include <cassert>
#include "utils/log.h"
#include "utils/fs.h"
#include "utils/string.h"
#include "core/const.h"
#include "core/idManager.h"
#include "core/patch.h"
#include "core/conf.h"
#include "core/plugin.h"
#include "pluginManager.h"


namespace giada {
namespace m {
namespace pluginManager
{
namespace
{
IdManager pluginId_;

int samplerate_;
int buffersize_;

/* formatManager
Plugin format manager. */

juce::AudioPluginFormatManager formatManager_;

/* knownPuginList
List of known (i.e. scanned) plugins. */

juce::KnownPluginList knownPluginList_;

/* unknownPluginList
List of unrecognized plugins found in a patch. */

std::vector<std::string> unknownPluginList_;

/* missingPlugins
If some plugins from any stack are missing. */

bool missingPlugins_;


std::unique_ptr<Plugin> makeInvalidPlugin_(const std::string& pid, ID id)
{
	missingPlugins_ = true;
	unknownPluginList_.push_back(pid);
	return std::make_unique<Plugin>(pluginId_.get(id), pid); // Invalid plug-in	
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void init(int samplerate, int buffersize)
{
	pluginId_       = IdManager();
	samplerate_     = samplerate;
    buffersize_     = buffersize;
	missingPlugins_ = false;

	formatManager_.addDefaultFormats();
	unknownPluginList_.clear();

	loadList(u::fs::getHomePath() + G_SLASH + "plugins.xml");
	sortPlugins(static_cast<pluginManager::SortMethod>(conf::conf.pluginSortMethod));
}


/* -------------------------------------------------------------------------- */


int scanDirs(const std::string& dirs, const std::function<void(float)>& cb)
{
	u::log::print("[pluginManager::scanDir] requested directories: '%s'\n", dirs.c_str());
	u::log::print("[pluginManager::scanDir] current plugins: %d\n", knownPluginList_.getNumTypes());

	knownPluginList_.clear();   // clear up previous plugins

	std::vector<std::string> dirVec = u::string::split(dirs, ";");

	juce::FileSearchPath searchPath;
	for (const std::string& dir : dirVec)
		searchPath.add(juce::File(dir));

	for (int i = 0; i < formatManager_.getNumFormats(); i++) {

		juce::PluginDirectoryScanner scanner(knownPluginList_, *formatManager_.getFormat(i), searchPath, 
			/*recursive=*/true, juce::File());

		juce::String name;
		while (scanner.scanNextFile(false, name)) {
			u::log::print("[pluginManager::scanDir]   scanning '%s'\n", name.toRawUTF8());
			cb(scanner.getProgress());
		}
	}

	u::log::print("[pluginManager::scanDir] %d plugin(s) found\n", knownPluginList_.getNumTypes());
	return knownPluginList_.getNumTypes();
}


/* -------------------------------------------------------------------------- */


bool saveList(const std::string& filepath)
{
	bool out = knownPluginList_.createXml()->writeTo(juce::File(filepath));
	if (!out)
		u::log::print("[pluginManager::saveList] unable to save plugin list to %s\n", filepath.c_str());
	return out;
}


/* -------------------------------------------------------------------------- */


bool loadList(const std::string& filepath)
{
	std::unique_ptr<juce::XmlElement> elem(juce::XmlDocument::parse(juce::File(filepath)));
	if (elem == nullptr)
		return false;
	knownPluginList_.recreateFromXml(*elem);
	return true;
}


/* -------------------------------------------------------------------------- */


std::unique_ptr<Plugin> makePlugin(const std::string& pid, ID id)
{
	/* Plug-in ID generator is updated anyway, as we store Plugin objects also
	if they are in an invalid state. */
	
	pluginId_.set(id);

	const std::unique_ptr<juce::PluginDescription> pd = knownPluginList_.getTypeForIdentifierString(pid);
	if (pd == nullptr) {
		u::log::print("[pluginManager::makePlugin] no plugin found with pid=%s!\n", pid.c_str());
		return makeInvalidPlugin_(pid, id);
	}

	juce::String error;
	std::unique_ptr<juce::AudioPluginInstance> pi = formatManager_.createPluginInstance(*pd, samplerate_, buffersize_, error);
	if (pi == nullptr) {
		u::log::print("[pluginManager::makePlugin] unable to create instance with pid=%s! Error: %s\n", 
			pid.c_str(), error.toStdString().c_str());
		return makeInvalidPlugin_(pid, id);
	}

	u::log::print("[pluginManager::makePlugin] plugin instance with pid=%s created\n", pid.c_str());

	return std::make_unique<Plugin>(pluginId_.get(id), std::move(pi), samplerate_, buffersize_);
}


/* -------------------------------------------------------------------------- */


std::unique_ptr<Plugin> makePlugin(int index)
{
	juce::PluginDescription pd = knownPluginList_.getTypes()[index];
	
	if (pd.uid == 0) // Invalid
		return {};
	
	u::log::print("[pluginManager::makePlugin] plugin found, uid=%s, name=%s...\n",
		pd.createIdentifierString().toRawUTF8(), pd.name.toRawUTF8());
	
	return makePlugin(pd.createIdentifierString().toStdString());

}


/* -------------------------------------------------------------------------- */


std::unique_ptr<Plugin> makePlugin(const Plugin& src)
{
	std::unique_ptr<Plugin> p = makePlugin(src.getUniqueId());
	
	for (int i=0; i<src.getNumParameters(); i++)
		p->setParameter(i, src.getParameter(i));	

	return p;
}


/* -------------------------------------------------------------------------- */


const patch::Plugin serializePlugin(const Plugin& p)
{
	patch::Plugin pp;
	pp.id     = p.id;
	pp.path   = p.getUniqueId();
	pp.bypass = p.isBypassed();

	for (int i = 0; i < p.getNumParameters(); i++)
		pp.params.push_back(p.getParameter(i));

	for (uint32_t param : p.midiInParams)
		pp.midiInParams.push_back(param);

	return pp;
}


/* -------------------------------------------------------------------------- */


std::unique_ptr<Plugin> deserializePlugin(const patch::Plugin& p)
{
	std::unique_ptr<Plugin> plugin = makePlugin(p.path, p.id);
	if (!plugin->valid)
		return plugin; // Return invalid version
	
	/* Fill plug-in parameters. */

	plugin->setBypass(p.bypass);
	for (unsigned j=0; j<p.params.size(); j++)
		plugin->setParameter(j, p.params.at(j));

	/* Fill plug-in MidiIn parameters. Don't fill Channel::midiInParam if 
	Plugin::midiInParams are zero: it would wipe out the current default 0x0
	values. */
	
	if (!p.midiInParams.empty()) {
		plugin->midiInParams.clear();
		for (uint32_t midiInParam : p.midiInParams)
			plugin->midiInParams.emplace_back(midiInParam);
	}

	return plugin;
}


/* -------------------------------------------------------------------------- */


int countAvailablePlugins()
{
	return knownPluginList_.getNumTypes();
}


/* -------------------------------------------------------------------------- */


unsigned countUnknownPlugins()
{
	return unknownPluginList_.size();
}


/* -------------------------------------------------------------------------- */


PluginInfo getAvailablePluginInfo(int i)
{
	juce::PluginDescription pd = knownPluginList_.getTypes()[i];
	PluginInfo pi;
	pi.uid              = pd.fileOrIdentifier.toStdString();
	pi.name             = pd.descriptiveName.toStdString();
	pi.category         = pd.category.toStdString();
	pi.manufacturerName = pd.manufacturerName.toStdString();
	pi.format           = pd.pluginFormatName.toStdString();
	pi.isInstrument     = pd.isInstrument;
	return pi;
}


/* -------------------------------------------------------------------------- */


bool hasMissingPlugins()
{
	return missingPlugins_;
}


/* -------------------------------------------------------------------------- */


std::string getUnknownPluginInfo(int i)
{
	return unknownPluginList_.at(i);
}


/* -------------------------------------------------------------------------- */


bool doesPluginExist(const std::string& pid)
{
	return formatManager_.doesPluginStillExist(*knownPluginList_.getTypeForFile(pid));
}


/* -------------------------------------------------------------------------- */


void sortPlugins(SortMethod method)
{
	switch (method) {
		case SortMethod::NAME:
			knownPluginList_.sort(juce::KnownPluginList::SortMethod::sortAlphabetically, true);
			break;
		case SortMethod::CATEGORY:
			knownPluginList_.sort(juce::KnownPluginList::SortMethod::sortByCategory, true);
			break;
		case SortMethod::MANUFACTURER:
			knownPluginList_.sort(juce::KnownPluginList::SortMethod::sortByManufacturer, true);
			break;
		case SortMethod::FORMAT:
			knownPluginList_.sort(juce::KnownPluginList::SortMethod::sortByFormat, true);
			break;
	}
}
}}} // giada::m::pluginManager::


#endif // #ifdef WITH_VST
