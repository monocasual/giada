/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#include <deque>
#include "deps/juce-config.h"
#include "pluginHost.h"
#include "const.h"


namespace giada {
namespace m 
{
class Plugin
{
public:

	Plugin(ID id, const std::string& UID);
	Plugin(ID id, juce::AudioPluginInstance* p, double samplerate, int buffersize);
	Plugin(const Plugin& o);
	~Plugin();

	/* getUniqueId
	Returns a string-based UID. */

	std::string getUniqueId() const;
	std::string getName() const;
	bool hasEditor() const;
	int getNumParameters() const;
	float getParameter(int index) const;
	std::string getParameterName(int index) const;
	std::string getParameterText(int index) const;
	std::string getParameterLabel(int index) const;
	bool isSuspended() const;
	bool isBypassed() const;
	int getNumPrograms() const;
	int getCurrentProgram() const;
	std::string getProgramName(int index) const;
	void setParameter(int index, float value) const;
	void setCurrentProgram(int index) const;
	bool acceptsMidi() const;

	juce::AudioProcessorEditor* createEditor() const;

	/* process
	Process the plug-in with audio and MIDI data. The audio buffer is a reference:
	it has to be altered by the plug-in itself. Conversely, the MIDI buffer must
	be passed by copy: each plug-in must receive its own copy of the event set, so
	that any attempt to change/clear the MIDI buffer will only modify the local 
	copy. */

	void process(juce::AudioBuffer<float>& b, juce::MidiBuffer m);

	void setBypass(bool b);

	/* id
	Unique identifier. */

	ID id;

	/* midiInParams
	A list of midiIn hex values for parameter automation. Why not a vector? 
	Unfortunately std::atomic types are not copy-constructible, nor 
	copy-assignable: such type won't suit a std::vector. */

	std::deque<std::atomic<uint32_t>> midiInParams;
	
	/* valid
	A missing plug-in is loaded anyway, yet marked as 'invalid'. */

	bool valid;

private:

#ifdef G_OS_WINDOWS
	/* Fuck... */
	#undef IN
	#undef OUT
#endif

	enum class BusType 
	{ 
		IN = true, OUT = false 
	};

	juce::AudioProcessor::Bus* getMainBus(BusType b) const;

	/* countMainOutChannels
	Returns the current channel layout for the main output bus. */

	int countMainOutChannels() const;

	juce::AudioPluginInstance* m_plugin;
	juce::AudioBuffer<float>   m_buffer;

	std::atomic<bool> m_bypass;

	/* UID
	The original UID, used for missing plugins. */

	std::string m_UID;
};
}} // giada::m::

#endif

#endif // #ifdef WITH_VST
