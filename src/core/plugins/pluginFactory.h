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

#ifndef G_PLUGIN_FACTORY_H
#define G_PLUGIN_FACTORY_H

#include "core/patch.h"
#include "core/types.h"
#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <string>

namespace giada::m
{
class Plugin;
}

namespace giada::m::model
{
class Sequencer;
}

namespace giada::m::pluginFactory
{
/* reset
Resets internal ID generator. */

void reset();

std::unique_ptr<Plugin> createInvalid(const std::string& pid, ID id);
std::unique_ptr<Plugin> create(ID id, const std::string& pid, std::unique_ptr<juce::AudioPluginInstance>,
    const model::Sequencer&, int sampleRate, int bufferSize);

std::unique_ptr<Plugin> deserializePlugin(const Patch::Plugin&, std::unique_ptr<juce::AudioPluginInstance>,
    const model::Sequencer&, int sampleRate, int bufferSize);
} // namespace giada::m::pluginFactory

#endif
