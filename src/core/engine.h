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

#ifndef G_ENGINE_H
#define G_ENGINE_H

#include "core/actions/actionRecorder.h"
#include "core/actions/actions.h"
#include "core/api/IOApi.h"
#include "core/api/actionEditorApi.h"
#include "core/api/channelsApi.h"
#include "core/api/mainApi.h"
#include "core/api/pluginsApi.h"
#include "core/api/sampleEditorApi.h"
#include "core/api/storageApi.h"
#include "core/channels/channelFactory.h"
#include "core/channels/channelManager.h"
#include "core/eventDispatcher.h"
#include "core/init.h"
#include "core/jackTransport.h"
#include "core/kernelAudio.h"
#include "core/kernelMidi.h"
#include "core/midiDispatcher.h"
#include "core/midiMapper.h"
#include "core/midiSynchronizer.h"
#include "core/mixer.h"
#include "core/model/model.h"
#include "core/patch.h"
#include "core/plugins/pluginHost.h"
#include "core/plugins/pluginManager.h"
#include "core/recorder.h"
#include "core/sequencer.h"
#include "core/waveFactory.h"
#ifdef WITH_AUDIO_JACK
#include "core/jackSynchronizer.h"
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

	bool                             isAudioReady() const;
	bool                             hasAudioAPI(RtAudio::Api) const;
	bool                             hasMidiAPI(RtMidi::Api) const;
	RtAudio::Api                     getAudioAPI() const;
	std::vector<KernelAudio::Device> getAudioDevices() const;
	std::vector<std::string>         getMidiOutPorts() const;
	std::vector<std::string>         getMidiInPorts() const;
	int                              getSampleRate() const;
	int                              getBufferSize() const;
	const std::vector<std::string>&  getMidiMapFilesFound() const;

	/* getPatch
	Returns a read-only reference to the current loaded Patch. */

	const Patch& getPatch() const;

	const model::Layout& getLayout() const;

	void setLayout(const model::Layout&);

	/* init
	Initializes all sub-components. If KernelAudio fails to start, the process
	interrupts and Giada is put in an invalid state. */

	void init(const Conf&);

	/* reset
	Resets all sub-components to the initial state. Useful when Giada needs to
	be brought back to the startup state. */

	void reset(PluginManager::SortMethod);

	/* shutdown
	Closes the current audio device. */

	void shutdown(Conf&);

	/* suspend, resume
	Toggles Mixer's rendering operation. */

	void suspend();
	void resume();

#ifdef G_DEBUG_MODE
	void debug();
#endif

	MainApi&         getMainApi();
	ChannelsApi&     getChannelsApi();
	PluginsApi&      getPluginsApi();
	SampleEditorApi& getSampleEditorApi();
	ActionEditorApi& getActionEditorApi();
	IOApi&           getIOApi();
	StorageApi&      getStorageApi();

	/* get[... component ...]
	Returns a reference to an internal. TODO - these methods will be removed with
	new Channel rendering architecture */

	KernelMidi&             getKernelMidi();
	ActionRecorder&         getActionRecorder();
	PluginHost&             getPluginHost();
	MidiMapper<KernelMidi>& getMidiMapper();

	/* onMidi[Received|Sent]
	Callback fired when the engine has received or sent a MIDI event. */

	std::function<void()> onMidiReceived;
	std::function<void()> onMidiSent;

	/* onModelSwap
	Callback fired when the model gets swapped. */

	std::function<void(model::SwapType)> onModelSwap;

private:
	int  audioCallback(KernelAudio::CallbackInfo) const;
	void registerThread(Thread, bool isRealtime) const;

	Patch                  m_patch;
	model::Model           m_model;
	KernelAudio            m_kernelAudio;
	KernelMidi             m_kernelMidi;
	MidiMapper<KernelMidi> m_midiMapper;
	PluginHost             m_pluginHost;
	JackTransport          m_jackTransport;
	MidiSynchronizer       m_midiSynchronizer;
	Sequencer              m_sequencer;
	Mixer                  m_mixer;
	ChannelManager         m_channelManager;
	ActionRecorder         m_actionRecorder;
	Recorder               m_recorder;
	PluginManager          m_pluginManager;
	EventDispatcher        m_eventDispatcher;
	MidiDispatcher         m_midiDispatcher;
#ifdef WITH_AUDIO_JACK
	JackSynchronizer m_jackSynchronizer;
#endif

	MainApi         m_mainApi;
	ChannelsApi     m_channelsApi;
	PluginsApi      m_pluginsApi;
	SampleEditorApi m_sampleEditorApi;
	ActionEditorApi m_actionEditorApi;
	IOApi           m_ioApi;
	StorageApi      m_storageApi;
};
} // namespace giada::m

#endif
