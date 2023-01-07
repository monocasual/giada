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

#include "core/channelsEngine.h"
#include "core/channels/channelManager.h"
#include "core/engine.h"
#include "core/kernelAudio.h"
#include "core/midiSynchronizer.h"
#include "core/mixer.h"
#include "utils/fs.h"

namespace giada::m
{
ChannelsEngine::ChannelsEngine(Engine& e, KernelAudio& k, Mixer& m, Sequencer& s, ChannelManager& cm, Recorder& r, ActionRecorder& ar, PluginHost& ph, PluginManager& pm)
: m_engine(e)
, m_kernelAudio(k)
, m_mixer(m)
, m_sequencer(s)
, m_channelManager(cm)
, m_recorder(r)
, m_actionRecorder(ar)
, m_pluginHost(ph)
, m_pluginManager(pm)
{
}

/* -------------------------------------------------------------------------- */

bool ChannelsEngine::hasChannelsWithAudioData() const
{
	return m_channelManager.hasAudioData();
}

bool ChannelsEngine::hasChannelsWithActions() const
{
	return m_channelManager.hasActions();
}

/* -------------------------------------------------------------------------- */

Channel& ChannelsEngine::get(ID channelId)
{
	return m_channelManager.getChannel(channelId);
}

std::vector<Channel>& ChannelsEngine::getAll()
{
	return m_channelManager.getAllChannels();
}

/* -------------------------------------------------------------------------- */

Channel& ChannelsEngine::add(ID columnId, ChannelType type)
{
	const int position   = m_channelManager.getLastChannelPosition(columnId);
	const int bufferSize = m_kernelAudio.getBufferSize();
	return m_channelManager.addChannel(type, columnId, position, bufferSize);
}

int ChannelsEngine::loadSampleChannel(ID channelId, const std::string& filePath)
{
	const int                sampleRate  = m_kernelAudio.getSampleRate();
	const Resampler::Quality rsmpQuality = m_engine.conf.data.rsmpQuality;
	return m_channelManager.loadSampleChannel(channelId, filePath, sampleRate, rsmpQuality);
}

void ChannelsEngine::loadSampleChannel(ID channelId, Wave& wave)
{
	m_channelManager.loadSampleChannel(channelId, wave);
}

void ChannelsEngine::loadPreviewChannel(ID sourceChannelId)
{
	m_channelManager.loadWaveInPreviewChannel(sourceChannelId);
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::remove(ID channelId)
{
	const std::vector<Plugin*> plugins  = m_channelManager.getChannel(channelId).plugins;
	const bool                 hasSolos = m_channelManager.hasSolos();

	m_actionRecorder.clearChannel(channelId);
	m_channelManager.deleteChannel(channelId);
	m_mixer.updateSoloCount(hasSolos);

	/* Plug-in destruction must be done in the main thread, due to JUCE and 
	VST3 internal workings. */

	m_pluginHost.freePlugins(plugins);
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::freeSampleChannel(ID channelId)
{
	m_actionRecorder.clearChannel(channelId);
	m_channelManager.freeSampleChannel(channelId);
}

void ChannelsEngine::freePreviewChannel()
{
	m_channelManager.freeWaveInPreviewChannel();
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::clone(ID channelId)
{
	/* Plug-in cloning must be done in the main thread, due to JUCE and VST3 
	internal workings. */

	const Channel&             ch              = m_channelManager.getChannel(channelId);
	const int                  bufferSize      = m_kernelAudio.getBufferSize();
	const int                  patchSampleRate = m_engine.patch.data.samplerate;
	const std::vector<Plugin*> plugins         = m_pluginManager.clonePlugins(ch.plugins, patchSampleRate, bufferSize, m_engine.model);
	const ID                   nextChannelId   = channelFactory::getNextId();

	m_channelManager.cloneChannel(channelId, bufferSize, plugins);
	m_actionRecorder.cloneActions(channelId, nextChannelId);
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::move(ID channelId, ID columnId, int position)
{
	m_channelManager.moveChannel(channelId, columnId, position);
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::press(ID channelId, int velocity)
{
	const bool  canRecordActions = m_recorder.canRecordActions();
	const bool  canQuantize      = m_sequencer.canQuantize();
	const Frame currentFrameQ    = m_sequencer.getCurrentFrameQuantized();
	m_channelManager.keyPress(channelId, velocity, canRecordActions, canQuantize, currentFrameQ);
}

void ChannelsEngine::release(ID channelId)
{
	const bool  canRecordActions = m_recorder.canRecordActions();
	const Frame currentFrameQ    = m_sequencer.getCurrentFrameQuantized();
	m_channelManager.keyRelease(channelId, canRecordActions, currentFrameQ);
}

void ChannelsEngine::kill(ID channelId)
{
	const bool  canRecordActions = m_recorder.canRecordActions();
	const Frame currentFrameQ    = m_sequencer.getCurrentFrameQuantized();
	m_channelManager.keyKill(channelId, canRecordActions, currentFrameQ);
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::setVolume(ID channelId, float v)
{
	m_channelManager.setVolume(channelId, v);
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::setPitch(ID channelId, float v)
{
	m_channelManager.setPitch(channelId, v);
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::setPan(ID channelId, float v)
{
	m_channelManager.setPan(channelId, v);
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::toggleMute(ID channelId)
{
	m_channelManager.toggleMute(channelId);
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::toggleSolo(ID channelId)
{
	m_channelManager.toggleSolo(channelId);
	m_mixer.updateSoloCount(m_channelManager.hasSolos());
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::toggleArm(ID channelId)
{
	m_channelManager.toggleArm(channelId);
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::toggleReadActions(ID channelId)
{
	m_channelManager.toggleReadActions(channelId, m_sequencer.isRunning());
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::killReadActions(ID channelId)
{
	m_channelManager.killReadActions(channelId);
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::setInputMonitor(ID channelId, bool value)
{
	m_channelManager.setInputMonitor(channelId, value);
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::setOverdubProtection(ID channelId, bool value)
{
	m_channelManager.setOverdubProtection(channelId, value);
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::setSamplePlayerMode(ID channelId, SamplePlayerMode mode)
{
	m_channelManager.setSamplePlayerMode(channelId, mode);
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::setHeight(ID channelId, int h)
{
	m_channelManager.setHeight(channelId, h);
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::setName(ID channelId, const std::string& name)
{
	m_channelManager.renameChannel(channelId, name);
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::setPreviewTracker(Frame f)
{
	m_channelManager.setPreviewTracker(f);
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::clearAllActions(ID channelId)
{
	m_actionRecorder.clearChannel(channelId);
}

void ChannelsEngine::clearAllActions()
{
	m_actionRecorder.clearAllActions();
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::freeAllSampleChannels()
{
	m_channelManager.freeAllSampleChannels();
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::sendMidi(ID channelId, MidiEvent e)
{
	const bool  canRecordActions = m_recorder.canRecordActions();
	const Frame currentFrameQ    = m_sequencer.getCurrentFrameQuantized();
	m_channelManager.processMidiEvent(channelId, e, canRecordActions, currentFrameQ);
}

/* -------------------------------------------------------------------------- */

bool ChannelsEngine::saveSample(ID channelId, const std::string& filePath)
{
	if (!m_channelManager.saveSample(channelId, filePath))
		return false;
	m_engine.conf.data.samplePath = u::fs::dirname(filePath);
	return true;
}

/* -------------------------------------------------------------------------- */

Patch::Channel ChannelsEngine::serializeChannel(const Channel& ch)
{
	return channelFactory::serializeChannel(ch);
}

channelFactory::Data ChannelsEngine::deserializeChannel(const Patch::Channel& pch, float samplerateRatio, int bufferSize)
{
	const Resampler::Quality rsmpQuality = m_engine.conf.data.rsmpQuality;
	Wave*                    wave        = m_engine.model.findShared<Wave>(pch.waveId);
	std::vector<Plugin*>     plugins     = m_pluginManager.hydratePlugins(pch.pluginIds, m_engine.model);
	return channelFactory::deserializeChannel(pch, samplerateRatio, bufferSize, rsmpQuality, wave, plugins);
}
} // namespace giada::m
