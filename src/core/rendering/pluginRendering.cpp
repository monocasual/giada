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

#include "src/core/rendering/pluginRendering.h"
#include "src/core/plugins/pluginHost.h"

namespace giada::m::rendering
{
void renderAudioAndMidiPlugins(
    const model::ChannelView&                channelView,
    const model::Assets&                     assets,
    const juce::AudioPlayHead::PositionInfo& position,
    PluginHost&                              pluginHost)
{
	const model::Channel&            channel   = channelView.getChannel();
	const model::Channel::Rendering& rendering = channelView.getRendering();

	pluginHost.processStack(
	    rendering, channel.plugins, assets, position, &rendering.prepareMidiBuffer());
}

/* -------------------------------------------------------------------------- */

void renderAudioPlugins(
    const model::ChannelView&                channelView,
    const model::Assets&                     assets,
    const juce::AudioPlayHead::PositionInfo& position,
    PluginHost&                              pluginHost)
{
	const model::Channel& channel = channelView.getChannel();

	pluginHost.processStack(channelView.getRendering(), channel.plugins, assets, position, nullptr);
}
} // namespace giada::m::rendering
