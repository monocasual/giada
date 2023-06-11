/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "core/plugins/pluginManager.h"
#include "core/const.h"
#include "core/model/model.h"
#include "core/patch.h"
#include "core/plugins/plugin.h"
#include "core/plugins/pluginFactory.h"
#include "utils/fs.h"
#include "utils/log.h"
#include "utils/string.h"
#include <cassert>
#include <cstddef>
#include <memory>

namespace giada::m
{
void PluginManager::reset(SortMethod sortMethod)
{
	pluginFactory::reset();

	m_unknownPluginList.clear();
	if (m_formatManager.getNumFormats() == 0) // Must be called only once
		m_formatManager.addDefaultFormats();

	loadList(u::fs::join(u::fs::getHomePath(), "plugins.xml"));
	sortPlugins(sortMethod);
}

/* -------------------------------------------------------------------------- */

int PluginManager::scanDirs(const std::string& dirs, const std::function<void(float)>& cb)
{
	u::log::print("[pluginManager::scanDir] requested directories: '{}'\n", dirs);
	u::log::print("[pluginManager::scanDir] currently known plug-ins: {}\n", m_knownPluginList.getNumTypes());

	m_knownPluginList.clear(); // clear up previous plugins

	std::vector<std::string> dirVec = u::string::split(dirs, ";");

	juce::FileSearchPath searchPath;
	for (const std::string& dir : dirVec)
		searchPath.add(juce::File(dir));

	for (int i = 0; i < m_formatManager.getNumFormats(); i++)
	{
		juce::PluginDirectoryScanner scanner(m_knownPluginList, *m_formatManager.getFormat(i), searchPath,
		    /*recursive=*/true, juce::File());

		juce::String name;
		while (scanner.scanNextFile(false, name))
		{
			u::log::print("[pluginManager::scanDir]   scanning '{}'\n", name.toRawUTF8());
			cb(scanner.getProgress());
		}
	}

	u::log::print("[pluginManager::scanDir] {} plugin(s) found\n", m_knownPluginList.getNumTypes());
	return m_knownPluginList.getNumTypes();
}

/* -------------------------------------------------------------------------- */

bool PluginManager::saveList(const std::string& filepath) const
{
	bool out = m_knownPluginList.createXml()->writeTo(juce::File(filepath));
	if (!out)
		u::log::print("[pluginManager::saveList] unable to save plugin list to {}\n", filepath);
	return out;
}

/* -------------------------------------------------------------------------- */

bool PluginManager::loadList(const std::string& filepath)
{
	std::unique_ptr<juce::XmlElement> elem(juce::XmlDocument::parse(juce::File(filepath)));
	if (elem == nullptr)
		return false;
	m_knownPluginList.recreateFromXml(*elem);
	return true;
}

/* -------------------------------------------------------------------------- */

std::unique_ptr<Plugin> PluginManager::makePlugin(const std::string& pid,
    int sampleRate, int bufferSize, const model::Sequencer& sequencer, ID id)
{
	std::unique_ptr<juce::AudioPluginInstance> pi = makeJucePlugin(pid, sampleRate, bufferSize);
	return pluginFactory::create(id, pid, std::move(pi), sequencer, sampleRate, bufferSize);
}

/* -------------------------------------------------------------------------- */

std::unique_ptr<Plugin> PluginManager::makePlugin(int index, int sampleRate,
    int bufferSize, const model::Sequencer& sequencer)
{
	juce::PluginDescription pd = m_knownPluginList.getTypes()[index];

	if (pd.uniqueId == 0) // Invalid
		return {};

	u::log::print("[pluginManager::makePlugin] plugin found, uid={}, name={}...\n",
	    pd.createIdentifierString().toRawUTF8(), pd.name.toRawUTF8());

	return makePlugin(pd.createIdentifierString().toStdString(), sampleRate, bufferSize, sequencer);
}

/* -------------------------------------------------------------------------- */

std::unique_ptr<Plugin> PluginManager::makePlugin(const Plugin& src, int sampleRate,
    int bufferSize, const model::Sequencer& sequencer)
{
	std::unique_ptr<Plugin> p = makePlugin(src.getUniqueId(), sampleRate, bufferSize, sequencer);

	for (int i = 0; i < src.getNumParameters(); i++)
		p->setParameter(i, src.getParameter(i));

	return p;
}

/* -------------------------------------------------------------------------- */

const Patch::Plugin PluginManager::serializePlugin(const Plugin& p) const
{
	return pluginFactory::serializePlugin(p);
}

/* -------------------------------------------------------------------------- */

std::vector<Plugin*> PluginManager::clonePlugins(const std::vector<Plugin*>& source,
    int sampleRate, int bufferSize, model::Model& model)
{
	std::vector<Plugin*> clones;
	for (const Plugin* plugin : source)
		clones.push_back(&model.addShared(makePlugin(*plugin, sampleRate, bufferSize, model.get().sequencer)));

	return clones;
}

/* -------------------------------------------------------------------------- */

int PluginManager::countAvailablePlugins() const
{
	return m_knownPluginList.getNumTypes();
}

/* -------------------------------------------------------------------------- */

std::vector<PluginManager::PluginInfo> PluginManager::getPluginsInfo() const
{
	std::vector<PluginInfo> out;

	for (int i = 0; i < m_knownPluginList.getNumTypes(); i++)
	{
		juce::PluginDescription pd = m_knownPluginList.getTypes()[i];
		PluginInfo              pi;

		pi.uid              = pd.fileOrIdentifier.toStdString();
		pi.name             = pd.descriptiveName.toStdString();
		pi.category         = pd.category.toStdString();
		pi.manufacturerName = pd.manufacturerName.toStdString();
		pi.format           = pd.pluginFormatName.toStdString();
		pi.isInstrument     = pd.isInstrument;
		pi.exists           = m_formatManager.doesPluginStillExist(*m_knownPluginList.getTypeForFile(pi.uid));
		pi.isKnown          = true;
		out.push_back(pi);
	}

	for (const std::string& uid : m_unknownPluginList)
	{
		PluginInfo pi;
		pi.uid          = uid;
		pi.isInstrument = false;
		pi.exists       = false;
		pi.isKnown      = false;
		out.push_back(pi);
	}

	return out;
}

/* -------------------------------------------------------------------------- */

bool PluginManager::hasMissingPlugins() const
{
	return !m_unknownPluginList.empty();
}

/* -------------------------------------------------------------------------- */

void PluginManager::sortPlugins(SortMethod method)
{
	switch (method)
	{
	case SortMethod::NAME:
		m_knownPluginList.sort(juce::KnownPluginList::SortMethod::sortAlphabetically, true);
		break;
	case SortMethod::CATEGORY:
		m_knownPluginList.sort(juce::KnownPluginList::SortMethod::sortByCategory, true);
		break;
	case SortMethod::MANUFACTURER:
		m_knownPluginList.sort(juce::KnownPluginList::SortMethod::sortByManufacturer, true);
		break;
	case SortMethod::FORMAT:
		m_knownPluginList.sort(juce::KnownPluginList::SortMethod::sortByFormat, true);
		break;
	}
}

/* -------------------------------------------------------------------------- */

std::unique_ptr<juce::AudioPluginInstance> PluginManager::makeJucePlugin(const std::string& pid, int sampleRate, int bufferSize)
{
	const std::unique_ptr<juce::PluginDescription> pd = m_knownPluginList.getTypeForIdentifierString(pid);
	if (pd == nullptr)
	{
		u::log::print("[pluginManager::makeJucePlugin] no plugin found with pid={}!\n", pid);
		m_unknownPluginList.push_back(pid);
		return nullptr;
	}

	juce::String                               error;
	std::unique_ptr<juce::AudioPluginInstance> pi = m_formatManager.createPluginInstance(*pd, sampleRate, bufferSize, error);
	if (pi == nullptr)
	{
		u::log::print("[pluginManager::makeJucePlugin] unable to create instance with pid={}! Error: {}\n",
		    pid, error.toStdString());
		m_unknownPluginList.push_back(pid);
		return nullptr;
	}

	u::log::print("[pluginManager::makeJucePlugin] plugin instance with pid={} created\n", pid);

	return pi;
}
} // namespace giada::m