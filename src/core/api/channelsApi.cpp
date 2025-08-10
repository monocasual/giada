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

#include "src/core/api/channelsApi.h"
#include "src/core/channels/channelManager.h"
#include "src/core/engine.h"
#include "src/core/kernelAudio.h"
#include "src/core/midiSynchronizer.h"
#include "src/core/mixer.h"
#include "src/utils/fs.h"

namespace giada::m
{
ChannelsApi::ChannelsApi(model::Model& m, KernelAudio& k, Mixer& mx, Sequencer& s,
    ChannelManager& cm, Recorder& r, ActionRecorder& ar, PluginHost& ph, PluginManager& pm,
    rendering::Reactor& re)
: m_model(m)
, m_kernelAudio(k)
, m_mixer(mx)
, m_sequencer(s)
, m_channelManager(cm)
, m_recorder(r)
, m_actionRecorder(ar)
, m_pluginHost(ph)
, m_pluginManager(pm)
, m_reactor(re)
{
}

/* -------------------------------------------------------------------------- */

bool ChannelsApi::hasChannelsWithAudioData() const
{
	return m_channelManager.hasAudioData();
}

bool ChannelsApi::hasChannelsWithActions() const
{
	return m_channelManager.hasActions();
}

/* -------------------------------------------------------------------------- */

bool ChannelsApi::canRemoveTrack(std::size_t trackIndex) const
{
	return m_channelManager.canRemoveTrack(trackIndex);
}

/* -------------------------------------------------------------------------- */

Channel& ChannelsApi::get(ID channelId)
{
	return m_channelManager.getChannel(channelId);
}

/* -------------------------------------------------------------------------- */

model::Tracks& ChannelsApi::getTracks()
{
	return m_model.get().tracks;
}

/* -------------------------------------------------------------------------- */

void ChannelsApi::addTrack()
{
	const int bufferSize = m_kernelAudio.getBufferSize();
	m_channelManager.addTrack(bufferSize);
}

/* -------------------------------------------------------------------------- */

void ChannelsApi::removeTrack(std::size_t trackIndex)
{
	m_channelManager.removeTrack(trackIndex);
}

/* -------------------------------------------------------------------------- */

void ChannelsApi::setTrackWidth(std::size_t trackIndex, int width)
{
	m_channelManager.setTrackWidth(trackIndex, width);
}

/* -------------------------------------------------------------------------- */

Channel& ChannelsApi::add(ChannelType type, std::size_t trackIndex)
{
	const int bufferSize = m_kernelAudio.getBufferSize();
	return m_channelManager.addChannel(type, trackIndex, bufferSize);
}

/* -------------------------------------------------------------------------- */

void ChannelsApi::move(ID channelId, std::size_t newTrackIndex, std::size_t newPosition)
{
	m_channelManager.moveChannel(channelId, newTrackIndex, newPosition);
}

/* -------------------------------------------------------------------------- */

int ChannelsApi::loadSampleChannel(ID channelId, const std::string& filePath)
{
	const int                sampleRate  = m_kernelAudio.getSampleRate();
	const Resampler::Quality rsmpQuality = m_model.get().kernelAudio.rsmpQuality;
	return m_channelManager.loadSampleChannel(channelId, filePath, sampleRate, rsmpQuality, 0); // TODO - scene
}

void ChannelsApi::loadSampleChannel(ID channelId, Wave& wave)
{
	m_channelManager.loadSampleChannel(channelId, wave, 0); // TODO - scene
}

/* -------------------------------------------------------------------------- */

void ChannelsApi::remove(ID channelId)
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

void ChannelsApi::freeSampleChannel(ID channelId)
{
	m_actionRecorder.clearChannel(channelId);
	m_channelManager.freeSampleChannel(channelId, 0); // TODO - scene
}

/* -------------------------------------------------------------------------- */

void ChannelsApi::clone(ID channelId)
{
	/* Plug-in cloning must be done in the main thread, due to JUCE and VST3
	internal workings. */

	const Channel&             ch            = m_channelManager.getChannel(channelId);
	const int                  bufferSize    = m_kernelAudio.getBufferSize();
	const int                  sampleRate    = m_kernelAudio.getSampleRate();
	const std::vector<Plugin*> plugins       = m_pluginManager.clonePlugins(ch.plugins, sampleRate, bufferSize, m_model);
	const ID                   nextChannelId = channelFactory::getNextId();

	m_channelManager.cloneChannel(channelId, bufferSize, plugins);
	if (ch.hasActions)
		m_actionRecorder.cloneActions(channelId, nextChannelId);
}

/* -------------------------------------------------------------------------- */

void ChannelsApi::press(ID channelId, float velocity)
{
	const bool  canRecordActions = m_recorder.canRecordActions();
	const bool  canQuantize      = m_sequencer.canQuantize();
	const Frame currentFrameQ    = m_sequencer.getCurrentFrameQuantized();
	m_reactor.keyPress(channelId, velocity, canRecordActions, canQuantize, currentFrameQ);
}

void ChannelsApi::release(ID channelId)
{
	const bool  canRecordActions = m_recorder.canRecordActions();
	const Frame currentFrameQ    = m_sequencer.getCurrentFrameQuantized();
	m_reactor.keyRelease(channelId, canRecordActions, currentFrameQ);
}

void ChannelsApi::kill(ID channelId)
{
	const bool  canRecordActions = m_recorder.canRecordActions();
	const Frame currentFrameQ    = m_sequencer.getCurrentFrameQuantized();
	m_reactor.keyKill(channelId, canRecordActions, currentFrameQ);
}

/* -------------------------------------------------------------------------- */

void ChannelsApi::setVolume(ID channelId, float v)
{
	m_channelManager.setVolume(channelId, v);
}

/* -------------------------------------------------------------------------- */

void ChannelsApi::setPitch(ID channelId, float v)
{
	m_channelManager.setPitch(channelId, v);
}

/* -------------------------------------------------------------------------- */

void ChannelsApi::setPan(ID channelId, float v)
{
	m_channelManager.setPan(channelId, v);
}

/* -------------------------------------------------------------------------- */

void ChannelsApi::toggleMute(ID channelId)
{
	m_reactor.toggleMute(channelId);
}

/* -------------------------------------------------------------------------- */

void ChannelsApi::toggleSolo(ID channelId)
{
	m_reactor.toggleSolo(channelId);
	m_mixer.updateSoloCount(m_channelManager.hasSolos());
}

/* -------------------------------------------------------------------------- */

void ChannelsApi::toggleArm(ID channelId)
{
	m_channelManager.toggleArm(channelId);
}

/* -------------------------------------------------------------------------- */

void ChannelsApi::toggleReadActions(ID channelId)
{
	m_reactor.toggleReadActions(channelId, m_sequencer.isRunning());
}

/* -------------------------------------------------------------------------- */

void ChannelsApi::killReadActions(ID channelId)
{
	m_reactor.killReadActions(channelId);
}

/* -------------------------------------------------------------------------- */

void ChannelsApi::setInputMonitor(ID channelId, bool value)
{
	m_channelManager.setInputMonitor(channelId, value);
}

/* -------------------------------------------------------------------------- */

void ChannelsApi::setOverdubProtection(ID channelId, bool value)
{
	m_channelManager.setOverdubProtection(channelId, value);
}

/* -------------------------------------------------------------------------- */

void ChannelsApi::setSamplePlayerMode(ID channelId, SamplePlayerMode mode)
{
	m_channelManager.setSamplePlayerMode(channelId, mode);
}

/* -------------------------------------------------------------------------- */

void ChannelsApi::setHeight(ID channelId, int h)
{
	m_channelManager.setHeight(channelId, h);
}

/* -------------------------------------------------------------------------- */

void ChannelsApi::setName(ID channelId, const std::string& name)
{
	m_channelManager.renameChannel(channelId, name);
}

/* -------------------------------------------------------------------------- */

void ChannelsApi::setSendToMaster(ID channelId, bool value)
{
	m_channelManager.setSendToMaster(channelId, value);
}

/* -------------------------------------------------------------------------- */

void ChannelsApi::addExtraOutput(ID channelId, int offset)
{
	m_channelManager.addExtraOutput(channelId, offset);
}

void ChannelsApi::removeExtraOutput(ID channelId, std::size_t index)
{
	m_channelManager.removeExtraOutput(channelId, index);
}

/* -------------------------------------------------------------------------- */

void ChannelsApi::clearAllActions(ID channelId)
{
	m_actionRecorder.clearChannel(channelId);
}

void ChannelsApi::clearAllActions()
{
	m_actionRecorder.clearAllActions();
}

/* -------------------------------------------------------------------------- */

void ChannelsApi::freeAllSampleChannels()
{
	m_channelManager.freeAllSampleChannels(0); // TODO - scene
}

/* -------------------------------------------------------------------------- */

void ChannelsApi::sendMidi(ID channelId, const MidiEvent& e)
{
	const bool  canRecordActions = m_recorder.canRecordActions();
	const Frame currentFrameQ    = m_sequencer.getCurrentFrameQuantized();
	m_reactor.processMidiEvent(channelId, e, canRecordActions, currentFrameQ);
}

/* -------------------------------------------------------------------------- */

bool ChannelsApi::saveSample(ID channelId, const std::string& filePath)
{
	return m_channelManager.saveSample(channelId, filePath);
}
} // namespace giada::m
