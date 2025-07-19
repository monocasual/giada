/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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
#include "src/types.h"
#include <functional>
/* windows.h, included somewhere, defines 'small' as a macro and it clashes with
some enum defined in the JUCE GUI module. */
#ifdef G_OS_WINDOWS
#undef small
#endif
#include <juce_audio_basics/juce_audio_basics.h>
#include <juce_gui_basics/juce_gui_basics.h>
#include <memory>

namespace mcl
{
class AudioBuffer;
}

namespace giada::m
{
class Plugin;
}

namespace giada::m::model
{
class Model;
class Sequencer;
} // namespace giada::m::model

namespace giada::m
{
class PluginHost final
{
public:
	class Info final : public juce::AudioPlayHead
	{
	public:
		Info(const model::Sequencer&, int sampleRate);

		juce::Optional<juce::AudioPlayHead::PositionInfo> getPosition() const override;
		bool                                              canControlTransport() override;

	private:
		const model::Sequencer& m_sequencer;
		int                     m_sampleRate;
	};

	PluginHost(model::Model&);

	/* reset
	Brings everything back to the initial state. */

	void reset(int bufferSize);

	/* setBufferSize
	Sets a new buffer size value for the internal audio buffer. Must be called
	only when mixer is disabled. */

	void setBufferSize(int);

	/* addPlugin
	Loads a new plugin into memory. Returns a reference to the newly created
	object. */

	const Plugin& addPlugin(std::unique_ptr<Plugin> p);

	/* processStack
	Applies the fx list to the buffer. */

	void processStack(mcl::AudioBuffer& outBuf, const std::vector<Plugin*>& plugins,
	    const juce::MidiBuffer* events = nullptr);

	/* swapPlugin
	Swaps plug-in 1 with plug-in 2 in the plug-in vector. */

	void swapPlugin(const m::Plugin& p1, const m::Plugin& p2, std::vector<Plugin*>& plugins);

	/* freePlugin.
	Unloads plugin from memory. */

	void freePlugin(const m::Plugin& plugin);

	/* freePlugins
	Unloads multiple plugins. Useful when freeing or deleting a channel. */

	void freePlugins(const std::vector<Plugin*>& plugins);

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

	/* juceToGiadaOutBuf
	Copies the private JUCE buffer to Giada buffer 'outBuf'. */

	void juceToGiadaOutBuf(mcl::AudioBuffer& outBuf) const;

	void processPlugins(const std::vector<Plugin*>&, const juce::MidiBuffer& events);

	void processPlugin(Plugin*, const juce::MidiBuffer& events);

	model::Model& m_model;

	juce::AudioBuffer<float> m_audioBuffer;
};
} // namespace giada::m

#endif