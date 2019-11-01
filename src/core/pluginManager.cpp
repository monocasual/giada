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

/* pluginFormat
Plugin format manager. */

juce::VSTPluginFormat pluginFormat_;

/* knownPuginList
List of known (i.e. scanned) plugins. */

juce::KnownPluginList knownPluginList_;

/* unknownPluginList
List of unrecognized plugins found in a patch. */

std::vector<std::string> unknownPluginList_;

/* missingPlugins
If some plugins from any stack are missing. */

bool missingPlugins_;

std::vector<std::string> splitPluginDescription_(const std::string& descr)
{
	// input:  VST-mda-Ambience-18fae2d2-6d646141  string
	// output: [2-------------] [1-----] [0-----]  vector.size() == 3
	
	std::vector<std::string> out;

	std::string chunk = "";
	int count = 2;
	for (int i=descr.length()-1; i >= 0; i--) {
		if (descr[i] == '-' && count != 0) {
			out.push_back(chunk);
			count--;
			chunk = "";
		}
		else
			chunk += descr[i];
	}
	out.push_back(chunk);

	return out;
}


/* findPluginDescription
Browses the list of known plug-ins until plug-in with id == 'id' is found.
Unfortunately knownPluginList_.getTypeForIdentifierString(id) doesn't work for
VSTs: their ID is based on the plug-in file location. E.g.:

	/home/vst/mdaAmbience.so      -> VST-mdaAmbience-18fae2d2-6d646141
	/home/vst-test/mdaAmbience.so -> VST-mdaAmbience-b328b2f6-6d646141

The following function simply drops the first hash code during comparison. */

const juce::PluginDescription* findPluginDescription_(const std::string& id)
{
	std::vector<std::string> idParts = splitPluginDescription_(id);

	for (const juce::PluginDescription* pd : knownPluginList_) {
		std::vector<std::string> tmpIdParts = splitPluginDescription_(pd->createIdentifierString().toStdString());
		if (idParts[0] == tmpIdParts[0] && idParts[2] == tmpIdParts[2])
			return pd;
	}
	return nullptr;
}
}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void init(int samplerate, int buffersize)
{
	pluginId_       = IdManager();
	samplerate_     = samplerate;
    buffersize_     = buffersize;
	missingPlugins_ = false;
	unknownPluginList_.clear();
	loadList(gu_getHomePath() + G_SLASH + "plugins.xml");
	sortPlugins(static_cast<pluginManager::SortMethod>(conf::pluginSortMethod));
}


/* -------------------------------------------------------------------------- */


int scanDirs(const std::string& dirs, const std::function<void(float)>& cb)
{
	gu_log("[pluginManager::scanDir] requested directories: '%s'\n", dirs.c_str());
	gu_log("[pluginManager::scanDir] current plugins: %d\n", knownPluginList_.getNumTypes());

	knownPluginList_.clear();   // clear up previous plugins

	std::vector<std::string> dirVec = u::string::split(dirs, ";");

	juce::VSTPluginFormat format;
	juce::FileSearchPath searchPath;
	for (const std::string& dir : dirVec)
		searchPath.add(juce::File(dir));

	juce::PluginDirectoryScanner scanner(knownPluginList_, format, searchPath, 
		/*recursive=*/true, juce::File());

	juce::String name;
	while (scanner.scanNextFile(false, name)) {
		gu_log("[pluginManager::scanDir]   scanning '%s'\n", name.toRawUTF8());
		cb(scanner.getProgress());
	}

	gu_log("[pluginManager::scanDir] %d plugin(s) found\n", knownPluginList_.getNumTypes());
	return knownPluginList_.getNumTypes();
}


/* -------------------------------------------------------------------------- */


int saveList(const std::string& filepath)
{
	int out = knownPluginList_.createXml()->writeToFile(juce::File(filepath), "");
	if (!out)
		gu_log("[pluginManager::saveList] unable to save plugin list to %s\n", filepath.c_str());
	return out;
}


/* -------------------------------------------------------------------------- */


int loadList(const std::string& filepath)
{
	std::unique_ptr<juce::XmlElement> elem(juce::XmlDocument::parse(juce::File(filepath)));
	if (elem == nullptr)
		return 0;
	knownPluginList_.recreateFromXml(*elem);
	return 1;
}


/* -------------------------------------------------------------------------- */


std::unique_ptr<Plugin> makePlugin(const std::string& fid, ID id)
{
	/* Plug-in ID generator is updated anyway, as we store Plugin objects also
	if they are in an invalid state. */
	
	pluginId_.set(id);

	const juce::PluginDescription* pd = findPluginDescription_(fid);
	if (pd == nullptr) {
		gu_log("[pluginManager::makePlugin] no plugin found with fid=%s!\n", fid.c_str());
		missingPlugins_ = true;
		unknownPluginList_.push_back(fid);
		return std::make_unique<Plugin>(pluginId_.get(id), fid); // Invalid plug-in
	}

	juce::AudioPluginInstance* pi = pluginFormat_.createInstanceFromDescription(*pd, samplerate_, buffersize_);
	if (pi == nullptr) {
		gu_log("[pluginManager::makePlugin] unable to create instance with fid=%s!\n", fid.c_str());
		missingPlugins_ = true;
		unknownPluginList_.push_back(fid);
		return std::make_unique<Plugin>(pluginId_.get(id), fid); // Invalid plug-in
	}
	gu_log("[pluginManager::makePlugin] plugin instance with fid=%s created\n", fid.c_str());

	return std::make_unique<Plugin>(pluginId_.get(id), pi, samplerate_, buffersize_);
}


/* -------------------------------------------------------------------------- */


std::unique_ptr<Plugin> makePlugin(int index)
{
	juce::PluginDescription* pd = knownPluginList_.getType(index);
	
	if (pd == nullptr) 
		return {};
	
	gu_log("[pluginManager::makePlugin] plugin found, uid=%s, name=%s...\n",
		pd->createIdentifierString().toRawUTF8(), pd->name.toRawUTF8());
	
	return makePlugin(pd->createIdentifierString().toStdString());

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


std::unique_ptr<Plugin> makePlugin(const patch::Plugin& p)
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
	juce::PluginDescription* pd = knownPluginList_.getType(i);
	PluginInfo pi;
	pi.uid              = pd->fileOrIdentifier.toStdString();
	pi.name             = pd->name.toStdString();
	pi.category         = pd->category.toStdString();
	pi.manufacturerName = pd->manufacturerName.toStdString();
	pi.format           = pd->pluginFormatName.toStdString();
	pi.isInstrument     = pd->isInstrument;
	return pi;
}


/* -------------------------------------------------------------------------- */


bool hasMissingPlugins()
{
	return missingPlugins_;
};


/* -------------------------------------------------------------------------- */


std::string getUnknownPluginInfo(int i)
{
	return unknownPluginList_.at(i);
}


/* -------------------------------------------------------------------------- */


bool doesPluginExist(const std::string& fid)
{
	return pluginFormat_.doesPluginStillExist(*knownPluginList_.getTypeForFile(fid));
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
}}}; // giada::m::pluginManager::


#endif // #ifdef WITH_VST
