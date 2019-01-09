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
#include "../utils/log.h"
#include "../utils/fs.h"
#include "../utils/string.h"
#include "const.h"
#include "plugin.h"
#include "pluginManager.h"


using std::vector;
using std::string;


namespace giada {
namespace m {
namespace pluginManager
{
namespace
{
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

vector<string> unknownPluginList_;

/* missingPlugins
If some plugins from any stack are missing. */

bool missingPlugins_;

vector<string> splitPluginDescription_(const string& descr)
{
	// input:  VST-mda-Ambience-18fae2d2-6d646141  string
	// output: [2-------------] [1-----] [0-----]  vector.size() == 3
	
	vector<string> out;

	string chunk = "";
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

const juce::PluginDescription* findPluginDescription_(const string& id)
{
	vector<string> idParts = splitPluginDescription_(id);

	for (const juce::PluginDescription* pd : knownPluginList_) {
		vector<string> tmpIdParts = splitPluginDescription_(pd->createIdentifierString().toStdString());
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
	samplerate_     = samplerate;
    buffersize_     = buffersize;
	missingPlugins_ = false;
	unknownPluginList_.clear();
	loadList(gu_getHomePath() + G_SLASH + "plugins.xml");
}


/* -------------------------------------------------------------------------- */


int scanDirs(const string& dirs, const std::function<void(float)>& cb)
{
	gu_log("[pluginManager::scanDir] requested directories: '%s'\n", dirs.c_str());
	gu_log("[pluginManager::scanDir] current plugins: %d\n", knownPluginList_.getNumTypes());

	knownPluginList_.clear();   // clear up previous plugins

	vector<string> dirVec = u::string::split(dirs, ";");

	juce::VSTPluginFormat format;
	juce::FileSearchPath searchPath;
	for (const string& dir : dirVec)
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


int saveList(const string& filepath)
{
	int out = knownPluginList_.createXml()->writeToFile(juce::File(filepath), "");
	if (!out)
		gu_log("[pluginManager::saveList] unable to save plugin list to %s\n", filepath.c_str());
	return out;
}


/* -------------------------------------------------------------------------- */


int loadList(const string& filepath)
{
	juce::XmlElement* elem = juce::XmlDocument::parse(juce::File(filepath));
	if (elem != nullptr) {
		knownPluginList_.recreateFromXml(*elem);
		delete elem;
		return 1;
	}
	return 0;
}


/* -------------------------------------------------------------------------- */


std::unique_ptr<Plugin> makePlugin(const string& fid)
{
	/* Initialize plugin. The default mode uses getTypeForIdentifierString, 
	falling back to  getTypeForFile (deprecated) for old patches (< 0.14.4). */

	const juce::PluginDescription* pd = findPluginDescription_(fid);
	if (pd == nullptr) {
		gu_log("[pluginManager::makePlugin] no plugin found with fid=%s! Trying with "
			"deprecated mode...\n", fid.c_str());
		pd = knownPluginList_.getTypeForFile(fid);
		if (pd == nullptr) {
			gu_log("[pluginManager::makePlugin] still nothing to do, returning unknown plugin\n");
			missingPlugins_ = true;
			unknownPluginList_.push_back(fid);
			return {};
		}
	}

	juce::AudioPluginInstance* pi = pluginFormat_.createInstanceFromDescription(*pd, samplerate_, buffersize_);
	if (!pi) {
		gu_log("[pluginManager::makePlugin] unable to create instance with fid=%s!\n", fid.c_str());
		missingPlugins_ = true;
		return {};
	}
	gu_log("[pluginManager::makePlugin] plugin instance with fid=%s created\n", fid.c_str());

	return std::move(std::make_unique<Plugin>(pi, samplerate_, buffersize_));
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

	return std::move(p);
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


string getUnknownPluginInfo(int i)
{
	return unknownPluginList_.at(i);
}


/* -------------------------------------------------------------------------- */


bool doesPluginExist(const string& fid)
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
