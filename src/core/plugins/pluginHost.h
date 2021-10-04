/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#ifdef WITH_VST

#ifndef G_PLUGIN_HOST_H
#define G_PLUGIN_HOST_H

#include "core/types.h"
#include "deps/juce-config.h"
#include <functional>

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
}

namespace giada::m
{
class Sequencer;
class PluginHost final
{
public:
	class Info final : public juce::AudioPlayHead
	{
	public:
		Info(const Sequencer&, int sampleRate);

		bool getCurrentPosition(CurrentPositionInfo& result) override;
		bool canControlTransport() override;

	private:
		const Sequencer& m_sequencer;
		int              m_sampleRate;
	};

	PluginHost(model::Model&);

	/* reset
	Brings everything back to the initial state. */

	void reset(int bufferSize);

	/* addPlugin
	Loads a new plugin into memory. Returns a reference to the newly created
	object. */

	const Plugin& addPlugin(std::unique_ptr<Plugin> p);

	/* processStack
	Applies the fx list to the buffer. */

	void processStack(mcl::AudioBuffer& outBuf, const std::vector<Plugin*>& plugins,
	    juce::MidiBuffer* events = nullptr);

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
	void giadaToJuceTempBuf(const mcl::AudioBuffer& outBuf);

	/* juceToGiadaOutBuf
	Converts buffer from Juce to Giada. A note for the future: if we overwrite (=) 
	(as we do now) it's SEND, if we add (+) it's INSERT. */

	void juceToGiadaOutBuf(mcl::AudioBuffer& outBuf) const;

	void processPlugins(const std::vector<Plugin*>& plugins, juce::MidiBuffer& events);

	model::Model& m_model;

	juce::AudioBuffer<float> m_audioBuffer;
};
} // namespace giada::m

#endif

#endif // #ifdef WITH_VST
