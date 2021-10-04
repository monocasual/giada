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

#ifndef G_PLUGIN_H
#define G_PLUGIN_H

#include "core/const.h"
#include "core/midiLearnParam.h"
#include "core/plugins/pluginHost.h"
#include "core/plugins/pluginState.h"
#include "deps/juce-config.h"
#include <vector>

namespace giada::m
{
class Plugin : private juce::ComponentListener
{
public:
	/* Plugin (1)
	Constructs an invalid plug-in. */

	Plugin(ID id, const std::string& UID);

	/* Plugin (2)
	Constructs a valid and working plug-in. */

	Plugin(ID  id, std::unique_ptr<juce::AudioPluginInstance>, std::unique_ptr<PluginHost::Info>,
	    double samplerate, int buffersize);

	Plugin(const Plugin& o) = delete;
	Plugin(Plugin&& o)      = delete;
	Plugin& operator=(const Plugin&) = delete;
	Plugin& operator=(Plugin&&) = delete;

	~Plugin();

	/* getUniqueId
	Returns a string-based UID. */

	std::string                 getUniqueId() const;
	std::string                 getName() const;
	bool                        hasEditor() const;
	int                         getNumParameters() const;
	float                       getParameter(int index) const;
	std::string                 getParameterName(int index) const;
	std::string                 getParameterText(int index) const;
	std::string                 getParameterLabel(int index) const;
	bool                        isSuspended() const;
	bool                        isBypassed() const;
	int                         getNumPrograms() const;
	int                         getCurrentProgram() const;
	std::string                 getProgramName(int index) const;
	void                        setParameter(int index, float value) const;
	void                        setCurrentProgram(int index) const;
	bool                        acceptsMidi() const;
	PluginState                 getState() const;
	juce::AudioProcessorEditor* createEditor() const;

	/* process
	Process the plug-in with audio and MIDI data. The audio buffer is a reference:
	it has to be altered by the plug-in itself. Conversely, the MIDI buffer must
	be passed by copy: each plug-in must receive its own copy of the event set, so
	that any attempt to change/clear the MIDI buffer will only modify the local 
	copy. */

	void process(juce::AudioBuffer<float>& b, juce::MidiBuffer m);

	void setState(PluginState p);
	void setBypass(bool b);

	/* id
	Unique identifier. */

	ID id;

	/* midiInParams
	A vector of MidiLearnParam's for controlling plug-in parameters with
	external hardware. */

	std::vector<MidiLearnParam> midiInParams;

	/* valid
	A missing plug-in is loaded anyway, yet marked as 'invalid'. */

	bool valid;

	std::function<void(int w, int h)> onEditorResize;

private:
#ifdef G_OS_WINDOWS
/* Fuck... */
#undef IN
#undef OUT
#endif

	enum class BusType
	{
		IN  = true,
		OUT = false
	};

	/* JUCE overrides. */

	void componentMovedOrResized(juce::Component& c, bool moved, bool resized) override;

	juce::AudioProcessor::Bus* getMainBus(BusType b) const;

	/* countMainOutChannels
	Returns the current channel layout for the main output bus. */

	int countMainOutChannels() const;

	std::unique_ptr<juce::AudioPluginInstance> m_plugin;
	std::unique_ptr<PluginHost::Info>          m_playHead;
	juce::AudioBuffer<float>                   m_buffer;

	std::atomic<bool> m_bypass;

	/* UID
	The original UID, used for missing plugins. */

	std::string m_UID;

	/* m_hasEditor
	Cached boolean value that tells if the plug-in has editor. Some plug-ins
	take ages to query it, better fetch the property during construction. */

	bool m_hasEditor;
};
} // namespace giada::m

#endif

#endif // #ifdef WITH_VST
