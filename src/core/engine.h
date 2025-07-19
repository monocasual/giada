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

#ifndef G_ENGINE_H
#define G_ENGINE_H

#include "src/core/actions/actionRecorder.h"
#include "src/core/api/IOApi.h"
#include "src/core/api/actionEditorApi.h"
#include "src/core/api/channelsApi.h"
#include "src/core/api/configApi.h"
#include "src/core/api/mainApi.h"
#include "src/core/api/pluginsApi.h"
#include "src/core/api/sampleEditorApi.h"
#include "src/core/api/storageApi.h"
#include "src/core/channels/channelFactory.h"
#include "src/core/channels/channelManager.h"
#include "src/core/eventDispatcher.h"
#include "src/core/init.h"
#include "src/core/jackTransport.h"
#include "src/core/kernelAudio.h"
#include "src/core/kernelMidi.h"
#include "src/core/midiDispatcher.h"
#include "src/core/midiMapper.h"
#include "src/core/midiSynchronizer.h"
#include "src/core/mixer.h"
#include "src/core/model/model.h"
#include "src/core/plugins/pluginHost.h"
#include "src/core/plugins/pluginManager.h"
#include "src/core/recorder.h"
#include "src/core/rendering/reactor.h"
#include "src/core/rendering/renderer.h"
#include "src/core/sequencer.h"
#include "src/core/waveFactory.h"
#ifdef WITH_AUDIO_JACK
#include "src/core/jackSynchronizer.h"
#endif

namespace giada::m
{
struct Conf;
class Engine final
{
public:
	/* Engine()
	Prepares all sub-components by constructing them and setting up the required
	callback for inter-component communication. It doesn't start the engine yet. */

	Engine();

	bool isAudioReady() const;

	/* init
	Initializes all sub-components. If KernelAudio fails to start, the process
	interrupts and Giada is put in an invalid state. */

	void init(const Conf&);

	/* reset
	Resets all sub-components to the initial state. Useful when Giada needs to
	be brought back to the startup state. */

	void reset();

	/* shutdown
	Closes the current audio device. */

	void shutdown(Conf&);

	/* suspend, resume
	Toggles Mixer's rendering operation. */

	void suspend();
	void resume();

#if G_DEBUG_MODE
	void debug();
#endif

	MainApi&         getMainApi();
	ChannelsApi&     getChannelsApi();
	PluginsApi&      getPluginsApi();
	SampleEditorApi& getSampleEditorApi();
	ActionEditorApi& getActionEditorApi();
	IOApi&           getIOApi();
	StorageApi&      getStorageApi();
	ConfigApi&       getConfigApi();

	/* get[... component ...]
	Returns a reference to an internal. TODO - these methods will be removed with
	new Channel rendering architecture */

	KernelMidi&             getKernelMidi();
	ActionRecorder&         getActionRecorder();
	PluginHost&             getPluginHost();
	MidiMapper<KernelMidi>& getMidiMapper();

	/* onMidi[Received|Sent]
	Callback fired when the engine has received or sent a MIDI event. */

	std::function<void()>   onMidiReceived;
	std::function<void()>   onMidiSent;
	std::function<void(ID)> onMidiSentFromChannel;

	/* onModelSwap
	Callback fired when the model gets swapped. */

	std::function<void(model::SwapType)> onModelSwap;

private:
	void registerThread(Thread, bool isRealtime) const;

	model::Model           m_model;
	KernelAudio            m_kernelAudio;
	KernelMidi             m_kernelMidi;
	MidiMapper<KernelMidi> m_midiMapper;
	PluginHost             m_pluginHost;
	JackTransport          m_jackTransport;
	MidiSynchronizer       m_midiSynchronizer;
	Sequencer              m_sequencer;
	Mixer                  m_mixer;
	ActionRecorder         m_actionRecorder;
	ChannelManager         m_channelManager;
	Recorder               m_recorder;
	PluginManager          m_pluginManager;
	EventDispatcher        m_eventDispatcher;
	MidiDispatcher         m_midiDispatcher;
#ifdef WITH_AUDIO_JACK
	JackSynchronizer m_jackSynchronizer;
#endif
	rendering::Renderer m_renderer;
	rendering::Reactor  m_reactor;

	MainApi         m_mainApi;
	ChannelsApi     m_channelsApi;
	PluginsApi      m_pluginsApi;
	SampleEditorApi m_sampleEditorApi;
	ActionEditorApi m_actionEditorApi;
	IOApi           m_ioApi;
	StorageApi      m_storageApi;
	ConfigApi       m_configApi;
};
} // namespace giada::m

#endif
