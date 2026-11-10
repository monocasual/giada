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

#ifndef G_PLUGIN_HOST_H
#define G_PLUGIN_HOST_H

#include "src/const.h"
#include "src/core/model/channel.h"
#include "src/core/model/types.h"
#include "src/types.h"
#include <functional>
/* windows.h, included somewhere, defines 'small' as a macro and it clashes with
some enum defined in the JUCE GUI module. */
#if G_OS_WINDOWS
#undef small
#endif
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>

namespace mcl
{
class AudioBuffer;
}

namespace giada::m::model
{
struct Assets;
class Plugin;
} // namespace giada::m::model

namespace giada::m
{
class PluginHost final
{
public:
	PluginHost(model::RealtimeModel&);

	/* reset
	Brings everything back to the initial state. */

	void reset(int bufferSize);

	/* setBufferSize
	Sets a new buffer size value for the internal audio buffer. Must be called
	only when mixer is disabled. */

	void setBufferSize(int);

	/* processStack. */

	void processStack(
	    const model::Channel::Rendering&,
	    const std::vector<ID>& pluginIds,
	    const model::Assets&,
	    const juce::AudioPlayHead::PositionInfo&,
	    const juce::MidiBuffer* events);

	/* freePlugin.
	Unloads plugin from memory. */

	void freePlugin(ID pluginId, ID channelId);

	/* freePlugins
	Unloads multiple plugins. Useful when freeing or deleting a channel. */

	void freePlugins(ID channelId);

	/* swapPlugins
	Swap plug-in 1 with plug-in 2 in the channel's plug-ins stack. */

	void swapPlugins(ID pluginId1, ID pluginId2, ID channelId);

	/* freeAllPlugins
	Just deletes everything. */

	void freeAllPlugins();

	void setPluginParameter(ID pluginId, int paramIndex, float value);
	void setPluginProgram(ID pluginId, int programIndex);
	void toggleBypass(ID pluginId);

private:
	/* giadaToJuceTempBuf
	Copies the Giada buffer 'outBuf' to the private JUCE buffer for local
	processing. */

	void giadaToJuceTempBuf(const mcl::AudioBuffer& outBuf);

	void processPlugin(const model::Plugin&, juce::MidiBuffer& events);

	model::RealtimeModel&            m_model;
	juce::AudioBuffer<float> m_audioBuffer;
};
} // namespace giada::m

#endif