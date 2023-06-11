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

std::unique_ptr<Plugin> createInvalid(const std::string& pid, ID id)
{
	return std::make_unique<Plugin>(pluginId_.generate(id), pid);
}

/* -------------------------------------------------------------------------- */

std::unique_ptr<Plugin> create(ID id, std::unique_ptr<juce::AudioPluginInstance> pi, const model::Sequencer& sequencer,
    int sampleRate, int bufferSize)
{
	return std::make_unique<Plugin>(
	    pluginId_.generate(id),
	    std::move(pi),
	    std::make_unique<PluginHost::Info>(sequencer, sampleRate),
	    sampleRate, bufferSize);
}
} // namespace giada::m::pluginFactory
