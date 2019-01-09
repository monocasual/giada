/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#include "../deps/juce-config.h"


namespace giada {
namespace m 
{
class Plugin
{
public:

	Plugin(juce::AudioPluginInstance* p, double samplerate, int buffersize);
	~Plugin();

	/* getUniqueId
	Returns a string-based UID. */

	std::string getUniqueId() const;

	std::string getName() const;
	bool isEditorOpen() const;
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
	int getId() const;
	int getEditorW() const;
	int getEditorH() const;
	void setParameter(int index, float value) const;
	void setCurrentProgram(int index) const;
	bool acceptsMidi() const;

	/* process
	Process the plug-in with audio and MIDI data. The audio buffer is a reference:
	it has to be altered by the plug-in itself. Conversely, the MIDI buffer must
	be passed by copy: each plug-in must receive its own copy of the event set, so
	that any attempt to change/clear the MIDI buffer will only modify the local 
	copy. */

	void process(juce::AudioBuffer<float>& b, juce::MidiBuffer m);

	void showEditor(void* parent);

	/* closeEditor
	Shuts down plugin GUI. */

	void closeEditor();

	void toggleBypass();
	void setBypass(bool b);

	/* midiInParams
	A list of midiIn hex values for parameter automation. */

	std::vector<uint32_t> midiInParams;

private:

	enum class BusType { IN = true, OUT = false };

	static const int MAX_LABEL_SIZE = 64;
	
	static int m_idGenerator;

	juce::AudioProcessorEditor* m_ui;     // gui
	juce::AudioPluginInstance*  m_plugin; // core
	juce::AudioBuffer<float>    m_buffer;

	int m_id;
	bool m_bypass;

	juce::AudioProcessor::Bus* getMainBus(BusType b) const;

	/* countMainOutChannels
	Returns the current channel layout for the main output bus. */

	int countMainOutChannels() const;
};

}} // giada::m::

#endif

#endif // #ifdef WITH_VST
