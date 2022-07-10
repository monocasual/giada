/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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
	m_pluginId       = IdManager();
	m_missingPlugins = false;

	m_unknownPluginList.clear();
	if (m_formatManager.getNumFormats() == 0) // Must be called only once
		m_formatManager.addDefaultFormats();

	loadList(u::fs::join(u::fs::getHomePath(), "plugins.xml"));
	sortPlugins(sortMethod);
}

/* -------------------------------------------------------------------------- */

int PluginManager::scanDirs(const std::string& dirs, const std::function<void(float)>& cb)
{
	u::log::print("[pluginManager::scanDir] requested directories: '%s'\n", dirs);
	u::log::print("[pluginManager::scanDir] currently known plug-ins: %d\n", m_knownPluginList.getNumTypes());

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
			u::log::print("[pluginManager::scanDir]   scanning '%s'\n", name.toRawUTF8());
			cb(scanner.getProgress());
		}
	}

	u::log::print("[pluginManager::scanDir] %d plugin(s) found\n", m_knownPluginList.getNumTypes());
	return m_knownPluginList.getNumTypes();
}

/* -------------------------------------------------------------------------- */

bool PluginManager::saveList(const std::string& filepath) const
{
	bool out = m_knownPluginList.createXml()->writeTo(juce::File(filepath));
	if (!out)
		u::log::print("[pluginManager::saveList] unable to save plugin list to %s\n", filepath);
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
    int sampleRate, int bufferSize, const Sequencer& sequencer, ID id)
{
	/* Plug-in ID generator is updated anyway, as we store Plugin objects also
	if they are in an invalid state. */

	m_pluginId.set(id);

	const std::unique_ptr<juce::PluginDescription> pd = m_knownPluginList.getTypeForIdentifierString(pid);
	if (pd == nullptr)
	{
		u::log::print("[pluginManager::makePlugin] no plugin found with pid=%s!\n", pid);
		return makeInvalidPlugin(pid, id);
	}

	juce::String                               error;
	std::unique_ptr<juce::AudioPluginInstance> pi = m_formatManager.createPluginInstance(*pd, sampleRate, bufferSize, error);
	if (pi == nullptr)
	{
		u::log::print("[pluginManager::makePlugin] unable to create instance with pid=%s! Error: %s\n",
		    pid, error.toStdString());
		return makeInvalidPlugin(pid, id);
	}

	u::log::print("[pluginManager::makePlugin] plugin instance with pid=%s created\n", pid);

	return std::make_unique<Plugin>(
	    m_pluginId.generate(id),
	    std::move(pi),
	    std::make_unique<PluginHost::Info>(sequencer, sampleRate),
	    sampleRate, bufferSize);
}

/* -------------------------------------------------------------------------- */

std::unique_ptr<Plugin> PluginManager::makePlugin(int index, int sampleRate,
    int bufferSize, const Sequencer& sequencer)
{
	juce::PluginDescription pd = m_knownPluginList.getTypes()[index];

	if (pd.uniqueId == 0) // Invalid
		return {};

	u::log::print("[pluginManager::makePlugin] plugin found, uid=%s, name=%s...\n",
	    pd.createIdentifierString().toRawUTF8(), pd.name.toRawUTF8());

	return makePlugin(pd.createIdentifierString().toStdString(), sampleRate, bufferSize, sequencer);
}

/* -------------------------------------------------------------------------- */

std::unique_ptr<Plugin> PluginManager::makePlugin(const Plugin& src, int sampleRate,
    int bufferSize, const Sequencer& sequencer)
{
	std::unique_ptr<Plugin> p = makePlugin(src.getUniqueId(), sampleRate, bufferSize, sequencer);

	for (int i = 0; i < src.getNumParameters(); i++)
		p->setParameter(i, src.getParameter(i));

	return p;
}

/* -------------------------------------------------------------------------- */

const Patch::Plugin PluginManager::serializePlugin(const Plugin& p) const
{
	Patch::Plugin pp;
	pp.id     = p.id;
	pp.path   = p.getUniqueId();
	pp.bypass = p.isBypassed();
	pp.state  = p.getState().asBase64();

	for (const MidiLearnParam& param : p.midiInParams)
		pp.midiInParams.push_back(param.getValue());

	return pp;
}

/* -------------------------------------------------------------------------- */

std::unique_ptr<Plugin> PluginManager::deserializePlugin(const Patch::Plugin& p,
    int sampleRate, int bufferSize, const Sequencer& sequencer)
{
	std::unique_ptr<Plugin> plugin = makePlugin(p.path, sampleRate, bufferSize, sequencer, p.id);
	if (!plugin->valid)
		return plugin; // Return invalid version

	plugin->setBypass(p.bypass);
	plugin->setState(PluginState(p.state));

	/* Fill plug-in MidiIn parameters. Don't fill Plugin::midiInParam if 
	Patch::midiInParams are zero: it would wipe out the current default 0x0
	values. */

	if (!p.midiInParams.empty())
	{
		plugin->midiInParams.clear();
		std::size_t paramIndex = 0;
		for (uint32_t midiInParam : p.midiInParams)
			plugin->midiInParams.emplace_back(midiInParam, paramIndex++);
	}

	return plugin;
}

/* -------------------------------------------------------------------------- */

std::vector<Plugin*> PluginManager::hydratePlugins(std::vector<ID> pluginIds, model::Model& model)
{
	std::vector<Plugin*> out;
	for (ID id : pluginIds)
	{
		Plugin* plugin = model.findShared<Plugin>(id);
		if (plugin != nullptr)
			out.push_back(plugin);
	}
	return out;
}

/* -------------------------------------------------------------------------- */

std::vector<Plugin*> PluginManager::clonePlugins(const std::vector<Plugin*>& source,
    int sampleRate, int bufferSize, model::Model& model, const Sequencer& sequencer)
{
	std::vector<Plugin*> clones;
	for (const Plugin* plugin : source)
	{
		model.addShared(makePlugin(*plugin, sampleRate, bufferSize, sequencer));
		clones.push_back(&model.backShared<Plugin>());
	}
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
	return m_missingPlugins;
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

std::unique_ptr<Plugin> PluginManager::makeInvalidPlugin(const std::string& pid, ID id)
{
	m_missingPlugins = true;
	m_unknownPluginList.push_back(pid);
	return std::make_unique<Plugin>(m_pluginId.generate(id), pid); // Invalid plug-in
}
} // namespace giada::m