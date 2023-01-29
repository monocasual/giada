/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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
#include "core/model/model.h"
#include "utils/fs.h"
#include "utils/log.h"
#include <fmt/core.h>
#include <memory>

namespace giada::m
{
Engine::Engine()
: midiMapper(m_kernelMidi)
, m_pluginHost(m_model)
, m_midiSynchronizer(conf.data, m_kernelMidi)
, m_sequencer(m_model, m_midiSynchronizer, m_jackTransport)
, m_mixer(m_model)
, m_channelManager(conf.data, m_model)
, m_actionRecorder(m_model)
, m_recorder(m_sequencer, m_channelManager, m_mixer, m_actionRecorder)
, m_midiDispatcher(m_model)
, m_mainEngine(*this, m_kernelAudio, m_mixer, m_sequencer, m_midiSynchronizer, m_channelManager, m_recorder)
, m_channelsEngine(*this, m_model, m_kernelAudio, m_mixer, m_sequencer, m_channelManager, m_recorder, m_actionRecorder, m_pluginHost, m_pluginManager)
, m_pluginsEngine(*this, m_kernelAudio, m_channelManager, m_pluginManager, m_pluginHost, m_model)
, m_sampleEditorEngine(*this, m_model, m_channelManager)
, m_actionEditorEngine(*this, m_model, m_sequencer, m_actionRecorder)
, m_ioEngine(m_model, m_midiDispatcher, conf.data)
, m_storageEngine(*this, m_model, conf, m_patch, m_pluginManager, m_midiSynchronizer, m_mixer, m_channelManager, m_kernelAudio, m_sequencer, m_actionRecorder)
{
	m_kernelAudio.onAudioCallback = [this](KernelAudio::CallbackInfo info) {
		return audioCallback(info);
	};

	m_kernelMidi.onMidiReceived = [this](const MidiEvent& e) {
		if (!m_model.registerThread(Thread::MIDI, /*realtime=*/false))
		{
			u::log::print("[Engine::m_kernelMidi.onMidiReceived] Can't register MIDI thread!\n");
			return;
		}
		m_midiDispatcher.dispatch(e);
		m_midiSynchronizer.receive(e, m_sequencer.getBeats());
	};

	m_midiDispatcher.onEventReceived = [this]() {
		m_recorder.startActionRecOnCallback();
	};

	m_midiSynchronizer.onChangePosition = [this](int beat) {
		m_mainEngine.goToBeat(beat);
	};
	m_midiSynchronizer.onChangeBpm = [this](float bpm) {
		m_mainEngine.setBpm(bpm);
	};
	m_midiSynchronizer.onStart = [this]() {
		m_mainEngine.startSequencer();
	};
	m_midiSynchronizer.onStop = [this]() {
		m_mainEngine.stopSequencer();
	};

	/* The following JackSynchronizer and Mixer callbacks are all fired by the
	realtime thread, so the actions are performed by pumping events into the 
	Event Dispatcher, rather than invoking them directly. This is done on 
	purpose: the callback might (and surely will) contain non-const operations
	on the m_model that the realtime thread cannot perform directly. */

#ifdef WITH_AUDIO_JACK
	m_jackSynchronizer.onJackRewind = [this]() {
		m_eventDispatcher.pumpEvent([this]() { m_sequencer.jack_rewind(); });
	};
	m_jackSynchronizer.onJackChangeBpm = [this](float bpm) {
		m_eventDispatcher.pumpEvent([this, bpm]() { m_sequencer.jack_setBpm(bpm, m_kernelAudio.getSampleRate()); });
	};
	m_jackSynchronizer.onJackStart = [this]() {
		m_eventDispatcher.pumpEvent([this]() { m_sequencer.jack_start(); });
	};
	m_jackSynchronizer.onJackStop = [this]() {
		m_eventDispatcher.pumpEvent([this]() { m_sequencer.jack_stop(); });
	};
#endif

	m_mixer.onSignalTresholdReached = [this]() {
		m_eventDispatcher.pumpEvent([this]() { m_recorder.startInputRecOnCallback(); });
	};
	m_mixer.onEndOfRecording = [this]() {
		if (m_mixer.isRecordingInput())
			m_eventDispatcher.pumpEvent([this]() { m_recorder.stopInputRec(conf.data.inputRecMode, m_kernelAudio.getSampleRate()); });
	};

	m_channelManager.onChannelsAltered = [this]() {
		if (!m_recorder.canEnableFreeInputRec())
			conf.data.inputRecMode = InputRecMode::RIGID;
	};
	m_channelManager.onChannelRecorded = [this](Frame recordedFrames) {
		std::string filename = fmt::format("TAKE-{}.wav", m_patch.data.lastTakeId++);
		return waveFactory::createEmpty(recordedFrames, G_MAX_IO_CHANS, m_kernelAudio.getSampleRate(), filename);
	};

	m_sequencer.onAboutStart = [this](SeqStatus status) {
		/* TODO move this logic to Recorder */
		if (status == SeqStatus::WAITING)
			m_recorder.stopActionRec();
		conf.data.recTriggerMode = RecTriggerMode::NORMAL;
	};
	m_sequencer.onAboutStop = [this]() {
		/* If recordings (both input and action) are active deactivate them, but 
	store the takes. RecManager takes care of it. */
		/* TODO move this logic to Recorder */
		if (m_mixer.isRecordingActions())
			m_recorder.stopActionRec();
		else if (m_mixer.isRecordingInput())
			m_recorder.stopInputRec(conf.data.inputRecMode, m_kernelAudio.getSampleRate());
	};
	m_sequencer.onBpmChange = [this](float oldVal, float newVal, int quantizerStep) {
		m_actionRecorder.updateBpm(oldVal / newVal, quantizerStep);
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

const Patch::Data& Engine::getPatch() const
{
	return m_patch.data;
}

/* -------------------------------------------------------------------------- */

void Engine::updateMixerModel()
{
	m_model.get().mixer.limitOutput     = conf.data.limitOutput;
	m_model.get().mixer.allowsOverdub   = conf.data.inputRecMode == InputRecMode::RIGID;
	m_model.get().mixer.maxFramesToRec  = conf.data.inputRecMode == InputRecMode::FREE ? m_sequencer.getMaxFramesInLoop(m_kernelAudio.getSampleRate()) : m_sequencer.getFramesInLoop();
	m_model.get().mixer.recTriggerLevel = conf.data.recTriggerLevel;
	m_model.swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

void Engine::init()
{
	if (!m_model.registerThread(Thread::MAIN, /*realtime=*/false))
	{
		u::log::print("[Engine::init] Can't register main thread!\n");
		return;
	}

	m_model.reset();

	if (!conf.read())
		u::log::print("[Engine::init] Can't read configuration file! Using default values\n");

	loadConfig();

	if (!u::log::init(conf.data.logMode))
		u::log::print("[Engine::init] log init failed! Using default stdout\n");

	init::printBuildInfo();

	midiMapper.init();
	if (midiMapper.read(conf.data.midiMapPath) != G_FILE_OK)
		u::log::print("[Engine::init] MIDI map read failed!\n");

	/* Initialize KernelAudio. If fails, interrupt the Engine initialization:
    Giada can't work without a functional KernelAudio. */

	m_kernelAudio.openDevice(conf.data);
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
	m_pluginManager.reset(conf.data.pluginSortMethod);

	m_mixer.enable();
	m_kernelAudio.startStream();

	m_kernelMidi.openOutDevice(conf.data.midiSystem, conf.data.midiPortOut);
	m_kernelMidi.openInDevice(conf.data.midiSystem, conf.data.midiPortIn);
	m_kernelMidi.logPorts();
	m_kernelMidi.start();

	midiMapper.sendInitMessages(midiMapper.currentMap);
	m_eventDispatcher.start();
	m_midiSynchronizer.startSendClock(G_DEFAULT_BPM);

	updateMixerModel();
}

/* -------------------------------------------------------------------------- */

void Engine::reset()
{
	/* Managers first, due to the internal ID numbering. */

	channelFactory::reset();
	waveFactory::reset();
	m_pluginManager.reset(conf.data.pluginSortMethod);

	/* Then all other components. */

	m_model.reset();
	m_mixer.reset(m_sequencer.getMaxFramesInLoop(m_kernelAudio.getSampleRate()), m_kernelAudio.getBufferSize());
	m_channelManager.reset(m_kernelAudio.getBufferSize());
	m_sequencer.reset(m_kernelAudio.getSampleRate());
	m_actionRecorder.reset();
	m_pluginHost.reset(m_kernelAudio.getBufferSize());
}

/* -------------------------------------------------------------------------- */

void Engine::shutdown()
{
	if (m_kernelAudio.isReady())
	{
		m_kernelAudio.closeDevice();
		u::log::print("[Engine::shutdown] KernelAudio closed\n");
		m_mixer.disable();
		u::log::print("[Engine::shutdown] Mixer closed\n");
	}

	storeConfig();

	if (!conf.write())
		u::log::print("[Engine::shutdown] error while saving configuration file!\n");
	else
		u::log::print("[Engine::shutdown] configuration saved\n");

	u::log::close();

	/* Currently the Engine is global/static, and so are all of its sub-components, 
	Model included. Some plug-ins (JUCE-based ones) crash hard on destructor when 
	deleted as a result of returning from main, so it's better to free them all first.
	TODO - investigate this! */

	m_pluginHost.freeAllPlugins();
}

/* -------------------------------------------------------------------------- */

int Engine::audioCallback(KernelAudio::CallbackInfo kernelInfo) const
{
	if (!m_model.registerThread(Thread::AUDIO, /*realtime=*/true))
	{
		u::log::print("[Engine::audioCallback] can't register realtime thread!\n");
		return 0;
	}

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

	/* Mixer disabled, nothing to do here. */

	if (!layout_RT.mixer.a_isActive())
		return 0;

#ifdef WITH_AUDIO_JACK
	if (kernelInfo.withJack)
		m_jackSynchronizer.recvJackSync(m_jackTransport.getState());
#endif

	/* If the m_sequencer is running, advance it first (i.e. parse it for events). 
	Also advance channels (i.e. let them react to m_sequencer events), only if the 
	layout is not locked: another thread might altering channel's data in the 
	meantime (e.g. Plugins or Waves). */

	if (layout_RT.sequencer.isRunning())
	{
		const Frame        currentFrame  = layout_RT.sequencer.a_getCurrentFrame();
		const Frame        bufferSize    = in.countFrames();
		const Frame        quantizerStep = m_sequencer.getQuantizerStep();            // TODO pass this to m_sequencer.advance - or better, Advancer class
		const Range<Frame> renderRange   = {currentFrame, currentFrame + bufferSize}; // TODO pass this to m_sequencer.advance - or better, Advancer class

		const Sequencer::EventBuffer& events = m_sequencer.advance(layout_RT.sequencer, bufferSize, kernelInfo.sampleRate, m_actionRecorder);
		m_sequencer.render(out);
		if (!layout_RT.locked)
			m_mixer.advanceChannels(events, layout_RT, renderRange, quantizerStep);
	}

	/* Then render Mixer: render channels, process I/O. */

	m_mixer.render(out, in, layout_RT);

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

void Engine::setOnModelSwapCb(std::function<void(m::model::SwapType)> f)
{
	m_model.onSwap = f;
}

/* -------------------------------------------------------------------------- */

void Engine::storeConfig()
{
	conf.data.midiInEnabled    = m_model.get().midiIn.enabled;
	conf.data.midiInFilter     = m_model.get().midiIn.filter;
	conf.data.midiInRewind     = m_model.get().midiIn.rewind;
	conf.data.midiInStartStop  = m_model.get().midiIn.startStop;
	conf.data.midiInActionRec  = m_model.get().midiIn.actionRec;
	conf.data.midiInInputRec   = m_model.get().midiIn.inputRec;
	conf.data.midiInMetronome  = m_model.get().midiIn.metronome;
	conf.data.midiInVolumeIn   = m_model.get().midiIn.volumeIn;
	conf.data.midiInVolumeOut  = m_model.get().midiIn.volumeOut;
	conf.data.midiInBeatDouble = m_model.get().midiIn.beatDouble;
	conf.data.midiInBeatHalf   = m_model.get().midiIn.beatHalf;
}

/* -------------------------------------------------------------------------- */

void Engine::loadConfig()
{
	m_model.get().midiIn.enabled    = conf.data.midiInEnabled;
	m_model.get().midiIn.filter     = conf.data.midiInFilter;
	m_model.get().midiIn.rewind     = conf.data.midiInRewind;
	m_model.get().midiIn.startStop  = conf.data.midiInStartStop;
	m_model.get().midiIn.actionRec  = conf.data.midiInActionRec;
	m_model.get().midiIn.inputRec   = conf.data.midiInInputRec;
	m_model.get().midiIn.metronome  = conf.data.midiInMetronome;
	m_model.get().midiIn.volumeIn   = conf.data.midiInVolumeIn;
	m_model.get().midiIn.volumeOut  = conf.data.midiInVolumeOut;
	m_model.get().midiIn.beatDouble = conf.data.midiInBeatDouble;
	m_model.get().midiIn.beatHalf   = conf.data.midiInBeatHalf;

	m_model.swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

MainEngine&         Engine::getMainEngine() { return m_mainEngine; }
ChannelsEngine&     Engine::getChannelsEngine() { return m_channelsEngine; }
PluginsEngine&      Engine::getPluginsEngine() { return m_pluginsEngine; }
SampleEditorEngine& Engine::getSampleEditorEngine() { return m_sampleEditorEngine; }
ActionEditorEngine& Engine::getActionEditorEngine() { return m_actionEditorEngine; }
IOEngine&           Engine::getIOEngine() { return m_ioEngine; }
StorageEngine&      Engine::getStorageEngine() { return m_storageEngine; }

/* -------------------------------------------------------------------------- */

KernelMidi&     Engine::getKernelMidi() { return m_kernelMidi; }
ActionRecorder& Engine::getActionRecorder() { return m_actionRecorder; }
PluginHost&     Engine::getPluginHost() { return m_pluginHost; }
} // namespace giada::m
