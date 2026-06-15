/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/core/plugins/pluginFactory.h"
#include "src/core/idManager.h"
#include "src/core/plugins/plugin.h"
#include "src/core/plugins/pluginHost.h"
#include "src/deps/mcl-utils/src/container.hpp"

namespace utils = mcl::utils;

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

std::unique_ptr<Plugin> createInvalid(ID id, const std::string& juceId)
{
	return std::make_unique<Plugin>(pluginId_.generate(id), juceId);
}

/* -------------------------------------------------------------------------- */

std::unique_ptr<Plugin> create(ID id, const std::string& juceId, std::unique_ptr<juce::AudioPluginInstance> pi,
    const model::Sequencer& sequencer, int sampleRate, int bufferSize)
{
	/* If the original juce::AudioPluginInstance is invalid, just return an
	invalid giada::m::Plugin object. This way we can keep track of invalid
	plug-ins. */

	if (pi == nullptr)
		return pluginFactory::createInvalid(id, juceId);

	return std::make_unique<Plugin>(
	    pluginId_.generate(id),
	    juceId,
	    std::move(pi),
	    std::make_unique<PluginHost::PluginAudioPlayHead>(sequencer, sampleRate),
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
		return pluginFactory::createInvalid(pplugin.id, pplugin.juceId);

	std::unique_ptr<Plugin> plugin = create(pplugin.id, pplugin.juceId, std::move(pi), sequencer, sampleRate, bufferSize);

	plugin->setBypass(pplugin.bypass);
	plugin->setState(PluginState(pplugin.state));

	for (const auto& [index, pparam] : utils::container::enumerate(pplugin.midiInParams))
		plugin->getParameters()[index].learnParam = MidiLearnParam(pparam, index);

	return plugin;
}

/* -------------------------------------------------------------------------- */

Patch::Plugin serializePlugin(const Plugin& p)
{
	Patch::Plugin pp;
	pp.id     = p.id;
	pp.juceId = p.getJuceId();
	pp.bypass = p.isBypassed();
	pp.state  = p.getState().asBase64();

	for (const PluginParameter& param : p.getParameters())
		pp.midiInParams.push_back(param.learnParam.getValue());

	return pp;
}
} // namespace giada::m::pluginFactory
