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

#include "core/engine.h"
#include "core/conf.h"
#include "core/confFactory.h"
#include "core/model/model.h"
#include "core/rendering/midiOutput.h"
#include "utils/fs.h"
#include "utils/log.h"
#include "utils/string.h"
#include <fmt/core.h>
#include <memory>

namespace giada::m
{
Engine::Engine()
: onMidiReceived(nullptr)
, onMidiSent(nullptr)
, onModelSwap(nullptr)
, m_kernelAudio(m_model)
, m_kernelMidi(m_model)
, m_midiMapper(m_kernelMidi)
, m_pluginHost(m_model)
, m_midiSynchronizer(m_kernelMidi)
, m_sequencer(m_model, m_midiSynchronizer, m_jackTransport)
, m_mixer(m_model)
, m_actionRecorder(m_model)
, m_channelManager(m_model, m_midiMapper, m_actionRecorder, m_kernelMidi)
, m_recorder(m_sequencer, m_channelManager, m_mixer, m_actionRecorder)
, m_midiDispatcher(m_model)
#ifdef WITH_AUDIO_JACK
, m_renderer(m_sequencer, m_mixer, m_pluginHost, m_jackSynchronizer, m_jackTransport, m_kernelMidi)
#else
, m_renderer(m_sequencer, m_mixer, m_pluginHost, m_kernelMidi)
#endif
, m_reactor(m_model, m_midiMapper, m_actionRecorder, m_kernelMidi)
, m_mainApi(m_kernelAudio, m_mixer, m_sequencer, m_midiSynchronizer, m_channelManager, m_recorder, m_reactor)
, m_channelsApi(m_model, m_kernelAudio, m_mixer, m_sequencer, m_channelManager, m_recorder, m_actionRecorder, m_pluginHost, m_pluginManager, m_reactor)
, m_pluginsApi(m_kernelAudio, m_pluginManager, m_pluginHost, m_model)
, m_sampleEditorApi(m_kernelAudio, m_model, m_channelManager)
, m_actionEditorApi(*this, m_sequencer, m_actionRecorder)
, m_ioApi(m_model, m_midiDispatcher)
, m_storageApi(*this, m_model, m_pluginManager, m_midiSynchronizer, m_mixer, m_channelManager, m_kernelAudio, m_sequencer, m_actionRecorder)
, m_configApi(m_model, m_kernelAudio, m_kernelMidi, m_midiMapper, m_midiSynchronizer)
{
	m_kernelAudio.onAudioCallback = [this](mcl::AudioBuffer& out, const mcl::AudioBuffer& in) {
		registerThread(Thread::AUDIO, /*realtime=*/true);
		m_renderer.render(out, in, m_model);
		return 0;
	};
	m_kernelAudio.onStreamAboutToOpen = [this]() {
		m_mixer.disable();
	};
	m_kernelAudio.onStreamOpened = [this]() {
#ifdef WITH_AUDIO_JACK
		if (m_kernelAudio.getAPI() == RtAudio::Api::UNIX_JACK)
			m_jackTransport.setHandle(m_kernelAudio.getJackHandle());
#endif
		const int sampleRate = m_kernelAudio.getSampleRate();
		const int bufferSize = m_kernelAudio.getBufferSize();
		m_mixer.reset(m_sequencer.getMaxFramesInLoop(sampleRate), bufferSize);
		m_channelManager.setBufferSize(bufferSize);
		m_sequencer.setSampleRate(sampleRate);
		m_pluginHost.setBufferSize(bufferSize);
		m_mixer.enable();
	};

	m_kernelMidi.onMidiReceived = [this](const MidiEvent& e) {
		assert(onMidiReceived != nullptr);

		registerThread(Thread::MIDI, /*realtime=*/false);
		m_midiDispatcher.dispatch(e);
		m_midiSynchronizer.receive(e, m_sequencer.getBeats());
		onMidiReceived();
	};
	m_kernelMidi.onMidiSent = [this]() {
		assert(onMidiSent != nullptr);
		onMidiSent();
	};

	m_midiDispatcher.onEventReceived = [this]() {
		m_recorder.startActionRecOnCallback();
	};

	m_midiSynchronizer.onChangePosition = [this](int beat) {
		m_mainApi.goToBeat(beat);
	};
	m_midiSynchronizer.onChangeBpm = [this](float bpm) {
		m_mainApi.setBpm(bpm);
	};
	m_midiSynchronizer.onStart = [this]() {
		m_mainApi.startSequencer();
	};
	m_midiSynchronizer.onStop = [this]() {
		m_mainApi.stopSequencer();
	};

	/* The following JackSynchronizer and Mixer callbacks are all fired by the
	realtime thread, so the actions are performed by pumping events into the
	Event Dispatcher, rather than invoking them directly. This is done on
	purpose: the callback might (and surely will) contain non-const operations
	on the m_model that the realtime thread cannot perform directly. */

#ifdef WITH_AUDIO_JACK
	m_jackSynchronizer.onJackRewind = [this]() {
		m_eventDispatcher.pumpEvent([this]() {
			registerThread(Thread::EVENTS, /*realtime=*/false);
			m_sequencer.jack_rewind();
		});
	};
	m_jackSynchronizer.onJackChangeBpm = [this](float bpm) {
		m_eventDispatcher.pumpEvent([this, bpm]() {
			registerThread(Thread::EVENTS, /*realtime=*/false);
			m_sequencer.jack_setBpm(bpm, m_kernelAudio.getSampleRate());
		});
	};
	m_jackSynchronizer.onJackStart = [this]() {
		m_eventDispatcher.pumpEvent([this]() {
			registerThread(Thread::EVENTS, /*realtime=*/false);
			m_sequencer.jack_start();
		});
	};
	m_jackSynchronizer.onJackStop = [this]() {
		m_eventDispatcher.pumpEvent([this]() {
			registerThread(Thread::EVENTS, /*realtime=*/false);
			m_sequencer.jack_stop();
		});
	};
#endif

	m_mixer.onSignalTresholdReached = [this]() {
		m_eventDispatcher.pumpEvent([this]() {
			registerThread(Thread::EVENTS, /*realtime=*/false);
			m_recorder.startInputRecOnCallback();
		});
	};
	m_mixer.onEndOfRecording = [this]() {
		if (m_mixer.isRecordingInput())
			m_eventDispatcher.pumpEvent([this]() {
				registerThread(Thread::EVENTS, /*realtime=*/false);
				m_recorder.stopInputRec(m_kernelAudio.getSampleRate());
			});
	};

	m_channelManager.onChannelPlayStatusChanged = [this](ID channelId, ChannelStatus status) {
		m_eventDispatcher.pumpEvent([this, channelId, status]() {
			registerThread(Thread::EVENTS, /*realtime=*/false);
			const Channel& ch = m_model.get().channels.get(channelId);
			if (ch.midiLightning.enabled)
				rendering::sendMidiLightningStatus(ch.id, ch.midiLightning, status, /*isAudible=*/true /* TODO!!! */, m_midiMapper);
		});
	};

	m_channelManager.onChannelsAltered = [this]() {
		if (!m_recorder.canEnableFreeInputRec())
			m_mixer.setInputRecMode(InputRecMode::RIGID);
	};
	m_channelManager.onChannelRecorded = [this](Frame recordedFrames) {
		return waveFactory::createEmpty(recordedFrames, G_MAX_IO_CHANS, m_kernelAudio.getSampleRate(), "TAKE");
	};

	m_sequencer.onAboutStart = [this](SeqStatus status) {
		/* TODO move this logic to Recorder */
		if (status == SeqStatus::WAITING)
			m_recorder.stopActionRec();
		m_model.get().mixer.recTriggerMode = RecTriggerMode::NORMAL;
	};
	m_sequencer.onAboutStop = [this]() {
		/* If recordings (both input and action) are active deactivate them, but
	store the takes. RecManager takes care of it. */
		/* TODO move this logic to Recorder */
		if (m_mixer.isRecordingActions())
			m_recorder.stopActionRec();
		else if (m_mixer.isRecordingInput())
			m_recorder.stopInputRec(m_kernelAudio.getSampleRate());
	};
	m_sequencer.onBpmChange = [this](float oldVal, float newVal, int quantizerStep) {
		m_actionRecorder.updateBpm(oldVal / newVal, quantizerStep);
	};

	m_model.onSwap = [this](model::SwapType t) {
		assert(onModelSwap != nullptr);
		onModelSwap(t);
	};

	rendering::registerOnSendMidiCb([this](ID channelId) {
		onMidiSentFromChannel(channelId);
	});
}

/* -------------------------------------------------------------------------- */

bool Engine::isAudioReady() const
{
	return m_kernelAudio.isReady();
}

/* -------------------------------------------------------------------------- */

void Engine::init(const Conf& conf)
{
	registerThread(Thread::MAIN, /*realtime=*/false);

	m_model.init();
	m_model.load(conf);

	const model::Document& document = m_model.get();

	m_kernelAudio.init();

	m_mixer.reset(m_sequencer.getMaxFramesInLoop(m_kernelAudio.getSampleRate()), m_kernelAudio.getBufferSize());
	m_channelManager.reset(m_kernelAudio.getBufferSize());
	m_sequencer.reset(m_kernelAudio.getSampleRate());
	m_pluginHost.reset(m_kernelAudio.getBufferSize());
	m_pluginManager.reset();

	m_mixer.enable();
	m_kernelAudio.startStream();

	m_kernelMidi.init();
	m_kernelMidi.start();

	m_midiMapper.init();
	m_midiMapper.read(document.kernelMidi.midiMapPath);
	m_midiMapper.sendInitMessages();

	m_eventDispatcher.start();
	m_midiSynchronizer.startSendClock(G_DEFAULT_BPM);
}

/* -------------------------------------------------------------------------- */

void Engine::reset()
{
	/* Managers first, due to the internal ID numbering. */

	channelFactory::reset();
	waveFactory::reset();
	m_pluginManager.reset();

	/* Then all other components. */

	const int sampleRate = m_kernelAudio.getSampleRate();
	const int bufferSize = m_kernelAudio.getBufferSize();

	m_model.reset();
	m_mixer.reset(m_sequencer.getMaxFramesInLoop(sampleRate), bufferSize);
	m_channelManager.reset(bufferSize);
	m_sequencer.reset(sampleRate);
	m_actionRecorder.reset();
	m_pluginHost.reset(bufferSize);
}

/* -------------------------------------------------------------------------- */

void Engine::shutdown(Conf& conf)
{
	if (m_kernelAudio.isReady())
	{
		m_kernelAudio.shutdown();
		u::log::print("[Engine::shutdown] KernelAudio closed\n");
		m_mixer.disable();
		u::log::print("[Engine::shutdown] Mixer closed\n");
	}

	m_model.store(conf);

	/* It's safer and cleaner to free all plug-ins before closing the app. Some
	would crash badly otherwise. */

	m_pluginHost.freeAllPlugins();
}

/* -------------------------------------------------------------------------- */

void Engine::suspend()
{
	m_mixer.disable();
}

void Engine::resume()
{
	m_mixer.enable();
}

/* -------------------------------------------------------------------------- */

#ifdef G_DEBUG_MODE
void Engine::debug()
{
	m_model.debug();
}
#endif

/* -------------------------------------------------------------------------- */

void Engine::registerThread(Thread t, bool isRealtime) const
{
	if (!m_model.registerThread(t, isRealtime))
	{
		u::log::print("[Engine::registerThread] Can't register thread {}! Aborting\n", u::string::toString(t));
		std::abort();
	}
}

/* -------------------------------------------------------------------------- */

MainApi&         Engine::getMainApi() { return m_mainApi; }
ChannelsApi&     Engine::getChannelsApi() { return m_channelsApi; }
PluginsApi&      Engine::getPluginsApi() { return m_pluginsApi; }
SampleEditorApi& Engine::getSampleEditorApi() { return m_sampleEditorApi; }
ActionEditorApi& Engine::getActionEditorApi() { return m_actionEditorApi; }
IOApi&           Engine::getIOApi() { return m_ioApi; }
StorageApi&      Engine::getStorageApi() { return m_storageApi; }
ConfigApi&       Engine::getConfigApi() { return m_configApi; }

/* -------------------------------------------------------------------------- */

KernelMidi&             Engine::getKernelMidi() { return m_kernelMidi; }
ActionRecorder&         Engine::getActionRecorder() { return m_actionRecorder; }
PluginHost&             Engine::getPluginHost() { return m_pluginHost; }
MidiMapper<KernelMidi>& Engine::getMidiMapper() { return m_midiMapper; }
} // namespace giada::m
