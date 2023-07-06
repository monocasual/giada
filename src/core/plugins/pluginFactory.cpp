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

#include "core/plugins/pluginFactory.h"
#include "core/idManager.h"
#include "core/plugins/plugin.h"
#include "core/plugins/pluginHost.h"

namespace giada::m::pluginFactory
{
namespace
{
IdManager pluginId_;
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void reset()
{
	pluginId_ = IdManager();
}

/* -------------------------------------------------------------------------- */

std::unique_ptr<Plugin> createInvalid(ID id, const std::string& pid)
{
	return std::make_unique<Plugin>(pluginId_.generate(id), pid);
}

/* -------------------------------------------------------------------------- */

std::unique_ptr<Plugin> create(ID id, const std::string& pid, std::unique_ptr<juce::AudioPluginInstance> pi,
    const model::Sequencer& sequencer, int sampleRate, int bufferSize)
{
	/* If the original juce::AudioPluginInstance is invalid, just return an
    invalid giada::m::Plugin object. This way we can keep track of invalid
    plug-ins. */

	if (pi == nullptr)
		return pluginFactory::createInvalid(id, pid);

	return std::make_unique<Plugin>(
	    pluginId_.generate(id),
	    std::move(pi),
	    std::make_unique<PluginHost::Info>(sequencer, sampleRate),
	    sampleRate, bufferSize);
}

/* -------------------------------------------------------------------------- */

std::unique_ptr<Plugin> deserializePlugin(const Patch::Plugin& pplugin, std::unique_ptr<juce::AudioPluginInstance> pi,
    const model::Sequencer& sequencer, int sampleRate, int bufferSize)
{
	/* If the original juce::AudioPluginInstance is invalid, just return an
	invalid giada::m::Plugin object. This way we can keep track of invalid
	plug-ins. */

	if (pi == nullptr)
		return pluginFactory::createInvalid(pplugin.id, pplugin.path);

	std::unique_ptr<Plugin> plugin = create(pplugin.id, pplugin.path, std::move(pi), sequencer, sampleRate, bufferSize);

	plugin->setBypass(pplugin.bypass);
	plugin->setState(PluginState(pplugin.state));

	/* Fill plug-in MidiIn parameters. Don't fill Plugin::midiInParam if 
	Patch::midiInParams are zero: it would wipe out the current default 0x0
	values. */

	if (!pplugin.midiInParams.empty())
	{
		plugin->midiInParams.clear();
		std::size_t paramIndex = 0;
		for (uint32_t midiInParam : pplugin.midiInParams)
			plugin->midiInParams.emplace_back(midiInParam, paramIndex++);
	}

	return plugin;
}

/* -------------------------------------------------------------------------- */

Patch::Plugin serializePlugin(const Plugin& p)
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
} // namespace giada::m::pluginFactory
