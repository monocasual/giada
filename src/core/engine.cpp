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

#include "core/engine.h"
#include "core/conf.h"
#include "core/confFactory.h"
#include "core/model/model.h"
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
, m_midiMapper(m_kernelMidi)
, m_pluginHost(m_model)
, m_midiSynchronizer(m_model, m_kernelMidi)
, m_sequencer(m_model, m_midiSynchronizer, m_jackTransport)
, m_mixer(m_model)
, m_channelManager(m_model)
, m_actionRecorder(m_model)
, m_recorder(m_sequencer, m_channelManager, m_mixer, m_actionRecorder)
, m_midiDispatcher(m_model)
, m_mainApi(*this, m_kernelAudio, m_mixer, m_sequencer, m_midiSynchronizer, m_channelManager, m_recorder)
, m_channelsApi(*this, m_model, m_kernelAudio, m_mixer, m_sequencer, m_channelManager, m_recorder, m_actionRecorder, m_pluginHost, m_pluginManager)
, m_pluginsApi(*this, m_kernelAudio, m_channelManager, m_pluginManager, m_pluginHost, m_model)
, m_sampleEditorApi(*this, m_model, m_channelManager)
, m_actionEditorApi(*this, m_model, m_sequencer, m_actionRecorder)
, m_ioApi(m_model, m_midiDispatcher)
, m_storageApi(*this, m_model, m_patch, m_pluginManager, m_midiSynchronizer, m_mixer, m_channelManager, m_kernelAudio, m_sequencer, m_actionRecorder)
{
	m_kernelAudio.onAudioCallback = [this](KernelAudio::CallbackInfo info) {
		return audioCallback(info);
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

	m_channelManager.onChannelsAltered = [this]() {
		if (!m_recorder.canEnableFreeInputRec())
			m_mixer.setInputRecMode(InputRecMode::RIGID);
	};
	m_channelManager.onChannelRecorded = [this](Frame recordedFrames) {
		std::string filename = fmt::format("TAKE-{}.wav", m_patch.lastTakeId++);
		return waveFactory::createEmpty(recordedFrames, G_MAX_IO_CHANS, m_kernelAudio.getSampleRate(), filename);
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
}

/* -------------------------------------------------------------------------- */

bool Engine::isAudioReady() const
{
	return m_kernelAudio.isReady();
}

/* -------------------------------------------------------------------------- */

RtAudio::Api Engine::getAudioAPI() const
{
	return m_kernelAudio.getAPI();
}

/* -------------------------------------------------------------------------- */

bool Engine::hasAudioAPI(RtAudio::Api api) const
{
	return m_kernelAudio.hasAPI(api);
}

/* -------------------------------------------------------------------------- */

bool Engine::hasMidiAPI(RtMidi::Api api) const
{
	return m_kernelMidi.hasAPI(api);
}

/* -------------------------------------------------------------------------- */

std::vector<KernelAudio::Device> Engine::getAudioDevices() const
{
	return m_kernelAudio.getDevices();
}

/* -------------------------------------------------------------------------- */

std::vector<std::string> Engine::getMidiOutPorts() const
{
	return m_kernelMidi.getOutPorts();
}

std::vector<std::string> Engine::getMidiInPorts() const
{
	return m_kernelMidi.getInPorts();
}

/* -------------------------------------------------------------------------- */

int Engine::getSampleRate() const
{
	return m_kernelAudio.getSampleRate();
}

int Engine::getBufferSize() const
{
	return m_kernelAudio.getBufferSize();
}

/* -------------------------------------------------------------------------- */

const std::vector<std::string>& Engine::getMidiMapFilesFound() const
{
	return m_midiMapper.getMapFilesFound();
}

/* -------------------------------------------------------------------------- */

const Patch& Engine::getPatch() const
{
	return m_patch;
}

const model::Layout& Engine::getLayout() const
{
	return m_model.get();
}

void Engine::setLayout(const model::Layout& layout)
{
	m_model.set(layout);
}

/* -------------------------------------------------------------------------- */

void Engine::init(const Conf& conf)
{
	registerThread(Thread::MAIN, /*realtime=*/false);

	m_model.init();
	m_model.load(conf);

	const model::Layout& layout = m_model.get();

	m_midiMapper.init();
	if (m_midiMapper.read(layout.kernelMidi.midiMapPath) != G_FILE_OK)
		u::log::print("[Engine::init] MIDI map read failed!\n");

	/* Initialize KernelAudio. If fails, interrupt the Engine initialization:
	Giada can't work without a working KernelAudio. */

	m_kernelAudio.init(layout.kernelAudio.soundSystem);
	m_kernelAudio.openDevice(layout.kernelAudio);
	if (!m_kernelAudio.isReady())
		return;

#ifdef WITH_AUDIO_JACK
	if (m_kernelAudio.getAPI() == RtAudio::Api::UNIX_JACK)
		m_jackTransport.setHandle(m_kernelAudio.getJackHandle());
#endif

	m_mixer.reset(m_sequencer.getMaxFramesInLoop(m_kernelAudio.getSampleRate()), m_kernelAudio.getBufferSize());
	m_channelManager.reset(m_kernelAudio.getBufferSize());
	m_sequencer.reset(m_kernelAudio.getSampleRate());
	m_pluginHost.reset(m_kernelAudio.getBufferSize());
	m_pluginManager.reset(conf.pluginSortMethod);

	m_mixer.enable();
	m_kernelAudio.startStream();

	m_kernelMidi.openOutDevice(layout.kernelMidi.system, layout.kernelMidi.portOut);
	m_kernelMidi.openInDevice(layout.kernelMidi.system, layout.kernelMidi.portIn);
	m_kernelMidi.logPorts();
	m_kernelMidi.start();

	m_midiMapper.sendInitMessages(m_midiMapper.currentMap);
	m_eventDispatcher.start();
	m_midiSynchronizer.startSendClock(G_DEFAULT_BPM);
}

/* -------------------------------------------------------------------------- */

void Engine::reset(PluginManager::SortMethod pluginSortMethod)
{
	/* Managers first, due to the internal ID numbering. */

	channelFactory::reset();
	waveFactory::reset();
	m_pluginManager.reset(pluginSortMethod);

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
		m_kernelAudio.closeDevice();
		u::log::print("[Engine::shutdown] KernelAudio closed\n");
		m_mixer.disable();
		u::log::print("[Engine::shutdown] Mixer closed\n");
	}

	m_model.store(conf);

	/* Currently the Engine is global/static, and so are all of its sub-components,
	Model included. Some plug-ins (JUCE-based ones) crash hard on destructor when
	deleted as a result of returning from main, so it's better to free them all first.
	TODO - investigate this! */

	m_pluginHost.freeAllPlugins();
}

/* -------------------------------------------------------------------------- */

int Engine::audioCallback(KernelAudio::CallbackInfo kernelInfo) const
{
	registerThread(Thread::AUDIO, /*realtime=*/true);

	mcl::AudioBuffer out(static_cast<float*>(kernelInfo.outBuf), kernelInfo.bufferSize, kernelInfo.channelsOutCount);
	mcl::AudioBuffer in;
	if (kernelInfo.channelsInCount > 0)
		in = mcl::AudioBuffer(static_cast<float*>(kernelInfo.inBuf), kernelInfo.bufferSize, kernelInfo.channelsInCount);

	/* Clean up output buffer before any rendering. Do this even if mixer is
	disabled to avoid audio leftovers during a temporary suspension (e.g. when
	loading a new patch). */

	out.clear();

	if (!kernelInfo.ready)
		return 0;

	/* Prepare the LayoutLock. From this point on (until out of scope) the
	Layout is locked for realtime rendering by the audio thread. Rendering
	functions must access the realtime layout coming from layoutLock.get(). */

	const model::LayoutLock layoutLock = m_model.get_RT();
	const model::Layout&    layout_RT  = layoutLock.get();
	const model::Mixer&     mixer      = layout_RT.mixer;
	const model::Sequencer& sequencer  = layout_RT.sequencer;
	const model::Channels&  channels   = layout_RT.channels;

	/* Mixer disabled, nothing to do here. */

	if (!mixer.a_isActive())
		return 0;

#ifdef WITH_AUDIO_JACK
	if (kernelInfo.withJack)
		m_jackSynchronizer.recvJackSync(m_jackTransport.getState());
#endif

	/* If the m_sequencer is running, advance it first (i.e. parse it for events).
	Also advance channels (i.e. let them react to m_sequencer events), only if the
	layout is not locked: another thread might altering channel's data in the
	meantime (e.g. Plugins or Waves). */

	if (sequencer.isRunning())
	{
		const Frame        currentFrame  = sequencer.a_getCurrentFrame();
		const int          bufferSize    = in.countFrames();
		const int          quantizerStep = m_sequencer.getQuantizerStep();            // TODO pass this to m_sequencer.advance - or better, Advancer class
		const Range<Frame> renderRange   = {currentFrame, currentFrame + bufferSize}; // TODO pass this to m_sequencer.advance - or better, Advancer class

		const Sequencer::EventBuffer& events = m_sequencer.advance(sequencer, bufferSize, kernelInfo.sampleRate, m_actionRecorder);
		m_sequencer.render(out);
		if (!layout_RT.locked)
			m_mixer.advanceChannels(events, channels, renderRange, quantizerStep);
	}

	/* Then render Mixer: render channels, process I/O. */

	const int maxFramesToRec = mixer.inputRecMode == InputRecMode::FREE ? sequencer.getMaxFramesInLoop(kernelInfo.sampleRate) : sequencer.framesInLoop;
	m_mixer.render(out, in, layout_RT, maxFramesToRec);

	return 0;
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
		u::log::print("[Engine::registerThread] Can't register thread %s! Aborting\n", u::string::toString(t).c_str());
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

/* -------------------------------------------------------------------------- */

KernelMidi&             Engine::getKernelMidi() { return m_kernelMidi; }
ActionRecorder&         Engine::getActionRecorder() { return m_actionRecorder; }
PluginHost&             Engine::getPluginHost() { return m_pluginHost; }
MidiMapper<KernelMidi>& Engine::getMidiMapper() { return m_midiMapper; }
} // namespace giada::m
