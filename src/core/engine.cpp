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
#include "core/model/storage.h"
#include "utils/fs.h"
#include "utils/log.h"
#include <fmt/core.h>
#include <memory>

namespace giada::m
{
bool LoadState::isGood() const
{
	return patch == G_FILE_OK && missingWaves.empty() && missingPlugins.empty();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Engine::Engine()
: midiMapper(m_kernelMidi)
, midiDispatcher(model)
, midiSynchronizer(conf.data, m_kernelMidi)
, sequencer(model, midiSynchronizer, jackTransport)
, m_pluginHost(model)
, m_mixer(model)
, m_channelManager(conf.data, model)
, m_actionRecorder(model)
, m_recorder(sequencer, m_channelManager, m_mixer, m_actionRecorder)
, m_mainEngine(*this, m_kernelAudio, m_mixer, sequencer, midiSynchronizer, m_channelManager, m_recorder)
, m_channelsEngine(*this, m_kernelAudio, m_mixer, sequencer, m_channelManager, m_recorder, m_actionRecorder, m_pluginHost, m_pluginManager)
, m_pluginsEngine(*this, m_kernelAudio, m_channelManager, m_pluginManager, m_pluginHost, model)
, m_sampleEditorEngine(*this, m_channelManager)
, m_actionEditorEngine(*this, sequencer, m_actionRecorder)
{
	m_kernelAudio.onAudioCallback = [this](KernelAudio::CallbackInfo info) {
		return audioCallback(info);
	};

	m_kernelMidi.onMidiReceived = [this](const MidiEvent& e) {
		if (!model.registerThread(Thread::MIDI, /*realtime=*/false))
		{
			u::log::print("[Engine::m_kernelMidi.onMidiReceived] Can't register MIDI thread!\n");
			return;
		}
		midiDispatcher.dispatch(e);
		midiSynchronizer.receive(e, sequencer.getBeats());
	};

	midiDispatcher.onEventReceived = [this]() {
		m_recorder.startActionRecOnCallback();
	};

	midiSynchronizer.onChangePosition = [this](int beat) {
		m_mainEngine.goToBeat(beat);
	};
	midiSynchronizer.onChangeBpm = [this](float bpm) {
		m_mainEngine.setBpm(bpm);
	};
	midiSynchronizer.onStart = [this]() {
		m_mainEngine.startSequencer();
	};
	midiSynchronizer.onStop = [this]() {
		m_mainEngine.stopSequencer();
	};

	/* The following JackSynchronizer and Mixer callbacks are all fired by the
	realtime thread, so the actions are performed by pumping events into the 
	Event Dispatcher, rather than invoking them directly. This is done on 
	purpose: the callback might (and surely will) contain non-const operations
	on the model that the realtime thread cannot perform directly. */

#ifdef WITH_AUDIO_JACK
	jackSynchronizer.onJackRewind = [this]() {
		m_eventDispatcher.pumpEvent([this]() { sequencer.jack_rewind(); });
	};
	jackSynchronizer.onJackChangeBpm = [this](float bpm) {
		m_eventDispatcher.pumpEvent([this, bpm]() { sequencer.jack_setBpm(bpm, m_kernelAudio.getSampleRate()); });
	};
	jackSynchronizer.onJackStart = [this]() {
		m_eventDispatcher.pumpEvent([this]() { sequencer.jack_start(); });
	};
	jackSynchronizer.onJackStop = [this]() {
		m_eventDispatcher.pumpEvent([this]() { sequencer.jack_stop(); });
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
		std::string filename = fmt::format("TAKE-{}.wav", patch.data.lastTakeId++);
		return waveFactory::createEmpty(recordedFrames, G_MAX_IO_CHANS, m_kernelAudio.getSampleRate(), filename);
	};

	sequencer.onAboutStart = [this](SeqStatus status) {
		/* TODO move this logic to Recorder */
		if (status == SeqStatus::WAITING)
			m_recorder.stopActionRec();
		conf.data.recTriggerMode = RecTriggerMode::NORMAL;
	};
	sequencer.onAboutStop = [this]() {
		/* If recordings (both input and action) are active deactivate them, but 
	store the takes. RecManager takes care of it. */
		/* TODO move this logic to Recorder */
		if (m_mixer.isRecordingActions())
			m_recorder.stopActionRec();
		else if (m_mixer.isRecordingInput())
			m_recorder.stopInputRec(conf.data.inputRecMode, m_kernelAudio.getSampleRate());
	};
	sequencer.onBpmChange = [this](float oldVal, float newVal, int quantizerStep) {
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

void Engine::updateMixerModel()
{
	model.get().mixer.limitOutput     = conf.data.limitOutput;
	model.get().mixer.allowsOverdub   = conf.data.inputRecMode == InputRecMode::RIGID;
	model.get().mixer.maxFramesToRec  = conf.data.inputRecMode == InputRecMode::FREE ? sequencer.getMaxFramesInLoop(m_kernelAudio.getSampleRate()) : sequencer.getFramesInLoop();
	model.get().mixer.recTriggerLevel = conf.data.recTriggerLevel;
	model.swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

void Engine::init()
{
	if (!model.registerThread(Thread::MAIN, /*realtime=*/false))
	{
		u::log::print("[Engine::init] Can't register main thread!\n");
		return;
	}

	model.reset();

	if (!conf.read())
		u::log::print("[Engine::init] Can't read configuration file! Using default values\n");

	model::load(conf.data, *this);

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
		jackTransport.setHandle(m_kernelAudio.getJackHandle());
#endif

	m_mixer.reset(sequencer.getMaxFramesInLoop(m_kernelAudio.getSampleRate()), m_kernelAudio.getBufferSize());
	m_channelManager.reset(m_kernelAudio.getBufferSize());
	sequencer.reset(m_kernelAudio.getSampleRate());
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
	midiSynchronizer.startSendClock(G_DEFAULT_BPM);

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

	model.reset();
	m_mixer.reset(sequencer.getMaxFramesInLoop(m_kernelAudio.getSampleRate()), m_kernelAudio.getBufferSize());
	m_channelManager.reset(m_kernelAudio.getBufferSize());
	sequencer.reset(m_kernelAudio.getSampleRate());
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

	model::store(conf.data, *this);
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
	if (!model.registerThread(Thread::AUDIO, /*realtime=*/true))
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

	const model::LayoutLock layoutLock = model.get_RT();
	const model::Layout&    layout_RT  = layoutLock.get();

	/* Mixer disabled, nothing to do here. */

	if (!layout_RT.mixer.a_isActive())
		return 0;

#ifdef WITH_AUDIO_JACK
	if (kernelInfo.withJack)
		jackSynchronizer.recvJackSync(jackTransport.getState());
#endif

	/* If the sequencer is running, advance it first (i.e. parse it for events). 
	Also advance channels (i.e. let them react to sequencer events), only if the 
	layout is not locked: another thread might altering channel's data in the 
	meantime (e.g. Plugins or Waves). */

	if (layout_RT.sequencer.isRunning())
	{
		const Frame        currentFrame  = layout_RT.sequencer.a_getCurrentFrame();
		const Frame        bufferSize    = in.countFrames();
		const Frame        quantizerStep = sequencer.getQuantizerStep();              // TODO pass this to sequencer.advance - or better, Advancer class
		const Range<Frame> renderRange   = {currentFrame, currentFrame + bufferSize}; // TODO pass this to sequencer.advance - or better, Advancer class

		const Sequencer::EventBuffer& events = sequencer.advance(layout_RT.sequencer, bufferSize, kernelInfo.sampleRate, m_actionRecorder);
		sequencer.render(out);
		if (!layout_RT.locked)
			m_mixer.advanceChannels(events, layout_RT, renderRange, quantizerStep);
	}

	/* Then render Mixer: render channels, process I/O. */

	m_mixer.render(out, in, layout_RT);

	return 0;
}

/* -------------------------------------------------------------------------- */

bool Engine::store(const std::string& projectName, const std::string& projectPath,
    const std::string& patchPath, std::function<void(float)> progress)
{
	progress(0.0f);

	if (!u::fs::mkdir(projectPath))
	{
		u::log::print("[Engine::store] Unable to make project directory!\n");
		return false;
	}

	u::log::print("[Engine::store] Project dir created: %s\n", projectPath);

	/* Update all existing file paths in Waves, so that they point to the project
	folder they belong to. */

	for (std::unique_ptr<Wave>& w : model.getAllShared<model::WavePtrs>())
	{
		w->setPath(waveFactory::makeUniqueWavePath(projectPath, *w, model.getAllShared<model::WavePtrs>()));
		waveFactory::save(*w, w->getPath()); // TODO - error checking
	}

	progress(0.3f);

	/* Write Model into Patch, then into file. */

	patch.data.name = projectName;
	model::store(patch.data, *this);

	progress(0.6f);

	if (!patch.write(patchPath))
		return false;

	/* Store the parent folder the project belongs to, in order to reuse it the 
	next time. */

	conf.data.patchPath = u::fs::getUpDir(u::fs::getUpDir(patchPath));

	u::log::print("[Engine::store] Project patch saved as %s\n", patchPath);

	progress(1.0f);

	return true;
}

/* -------------------------------------------------------------------------- */

LoadState Engine::load(const std::string& projectPath, const std::string& patchPath,
    std::function<void(float)> progress)
{
	u::log::print("[Engine::load] Load project from %s\n", projectPath);

	progress(0.0f);

	/* Suspend MIDI clock output (if enabled). */

	midiSynchronizer.stopSendClock();

	/* Read the selected project's patch. */

	patch.reset();
	if (int res = patch.read(patchPath, projectPath); res != G_FILE_OK)
		return {res};

	progress(0.3f);

	/* Then suspend Mixer, reset and fill the model. */

	m_mixer.disable();
	reset();
	LoadState state = m::model::load(patch.data, *this);

	progress(0.6f);

	/* Prepare the engine. Recorder has to recompute the actions positions if 
	the current samplerate != patch samplerate. Clock needs to update frames
	in sequencer. */

	m_mixer.updateSoloCount(m_channelManager.hasSolos());
	m_actionRecorder.updateSamplerate(m_kernelAudio.getSampleRate(), patch.data.samplerate);
	sequencer.recomputeFrames(m_kernelAudio.getSampleRate());
	m_mixer.allocRecBuffer(sequencer.getMaxFramesInLoop(m_kernelAudio.getSampleRate()));

	progress(0.9f);

	/* Store the parent folder the project belongs to, in order to reuse it the 
	next time. */

	conf.data.patchPath = u::fs::getUpDir(projectPath);

	/* Mixer is ready to go back online. */

	m_mixer.enable();

	/* Restore MIDI clock output. */

	midiSynchronizer.startSendClock(model.get().sequencer.bpm);

	progress(1.0f);

	state.patch = G_FILE_OK;
	return state;
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

MainEngine&         Engine::getMainEngine() { return m_mainEngine; }
ChannelsEngine&     Engine::getChannelsEngine() { return m_channelsEngine; }
PluginsEngine&      Engine::getPluginsEngine() { return m_pluginsEngine; }
SampleEditorEngine& Engine::getSampleEditorEngine() { return m_sampleEditorEngine; }
ActionEditorEngine& Engine::getActionEditorEngine() { return m_actionEditorEngine; }

/* -------------------------------------------------------------------------- */

KernelMidi&     Engine::getKernelMidi() { return m_kernelMidi; }
ActionRecorder& Engine::getActionRecorder() { return m_actionRecorder; }
PluginHost&     Engine::getPluginHost() { return m_pluginHost; }
} // namespace giada::m
