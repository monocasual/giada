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

#ifndef G_RENDERING_PLUGIN_RENDERING_H
#define G_RENDERING_PLUGIN_RENDERING_H

#include "core/channels/channelShared.h"

namespace giada::m
{
class PluginHost;
}

namespace giada::m::rendering
{
/* renderAudioAndMidiPlugins
Renders plug-ins using the shared juce::MidiBuffer for MIDI event rendering. It
renders normal audio plug-ins too. */

void renderAudioAndMidiPlugins(const Channel&, PluginHost&);

/* renderAudioPlugins
Renders audio-only plug-ins. */

void renderAudioPlugins(const Channel&, PluginHost&);
} // namespace giada::m::rendering

#endif
