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

#ifndef G_CHANNEL_MANAGER_H
#define G_CHANNEL_MANAGER_H

#include "src/core/midiMapper.h"
#include "src/core/resampler.h"
#include "src/core/types.h"
#include "src/deps/geompp/src/range.hpp"
#include "src/types.h"
#include <functional>
#include <map>
#include <memory>
#include <unordered_set>

namespace mcl
{
class AudioBuffer;
}

namespace giada::m::model
{
class Model;
}

namespace giada::m
{
class Channel;
struct ChannelShared;
class Wave;
class Plugin;
class MidiEvent;
class Engine;
class ActionRecorder;
class KernelMidi;
class ChannelManager final
{
public:
	friend Engine;

	ChannelManager(model::Model&, MidiMapper<KernelMidi>&, ActionRecorder&, KernelMidi&);

	/* getChannel
	Returns channel object by ID. */

	Channel& getChannel(ID);

	/* hasInputRecordableChannels
	Tells whether Mixer has one or more input-recordable channels. */

	bool hasInputRecordableChannels() const;

	/* hasActions
	True if at least one Channel has actions recorded in it. */

	bool hasActions() const;

	/* hasAudioData
	True if at least one Sample Channel has some audio recorded in it. */

	bool hasAudioData() const;

	/* hasSolos
	True if there are soloed channels. */

	bool hasSolos() const;

	/* canRemoveTrack
	True if a track contains only one Group Channel. */

	bool canRemoveTrack(std::size_t trackIndex) const;

	float getMasterInVol() const;
	float getMasterOutVol() const;

	/* reset
	Brings channels configuration back to the initial state: two I/O master
	channels, one preview channel. */

	void reset(Frame framesInBuffer);

	/* setBufferSize
	Sets a new buffer size value to all channels. Must be called only when
	mixer is disabled. */

	void setBufferSize(int);

	/* addTrack
	Adds a new empty track, containing only the default Channel Group. */

	void addTrack(int bufferSize);

	/* removeTracks
	Removes a track. Note: it must be empty, except for the default Channel
	Group. Raises assertion otherwise. */

	void removeTrack(std::size_t trackIndex);

	void setTrackWidth(std::size_t trackIndex, int width);

	/* addChannel
	Adds a new channel to the stack. */

	Channel& addChannel(ChannelType, std::size_t trackIndex, int bufferSize);

	/* loadSampleChannel (1)
	Creates a new Wave from a file path and loads it inside a Sample Channel. */

	int loadSampleChannel(ID channelId, const std::string&, int sampleRate, Resampler::Quality, std::size_t scene);

	/* loadSampleChannel (2)
	Loads an existing Wave inside a Sample Channel. */

	void loadSampleChannel(ID channelId, Wave&, std::size_t scene);

	/* freeChannel
	Unloads existing Wave from a Sample Channel. */

	void freeSampleChannel(ID channelId, std::size_t scene);
	void freeAllSampleChannels(std::size_t scene);

	void moveChannel(ID, std::size_t newTrackIndex, std::size_t newPosition);
	void deleteChannel(ID channelId);
	void renameChannel(ID channelId, const std::string& name);

	/* cloneChannel
	Creates a duplicate of Channel. Wants a vector of already cloned plug-ins. */

	void cloneChannel(ID channelId, int bufferSize, const std::vector<Plugin*>&);

	/* finalizeInputRec
	Fills armed Sample channel with audio data coming from an input recording
	session. */

	void finalizeInputRec(const mcl::AudioBuffer&, Frame recordedFrames, Frame currentFrame, std::size_t scene);

	/* finalizeActionRec
	Enable reading actions for Channels that have just been filled with actions
	after an action recording session. This will start reading actions right
	away, without checking whether conf::treatRecsAsLoops is enabled or not.
	Same thing for MIDI channels.  */

	void finalizeActionRec(const std::unordered_set<ID>&);

	void setInputMonitor(ID channelId, bool value);
	void setVolume(ID channelId, float value);
	void setPitch(ID channelId, float value);
	void setPan(ID channelId, float value);
	void setRange(ID channelId, SampleRange, std::size_t scene);
	void resetRange(ID channelId, std::size_t scene);
	void toggleArm(ID channelId);
	void setOverdubProtection(ID channelId, bool value);
	void setSamplePlayerMode(ID channelId, SamplePlayerMode);
	void setHeight(ID channelId, int height);
	void setSendToMaster(ID channelId, bool value);
	void addExtraOutput(ID channelId, int offset);
	void removeExtraOutput(ID channelId, std::size_t);
	void loadWaveInPreviewChannel(ID sourceChannelId);
	void freeWaveInPreviewChannel();
	void setPreviewTracker(Frame f);
	bool saveSample(ID channelId, const std::string& filePath);

	/* onChannelsAltered
	Fired when something is done on channels (added, removed, loaded, ...). */

	std::function<void()> onChannelsAltered;

	/* onChannelRecorded
	Fired during the input recording finalization, when a new empty Wave must
	be added to each armed channel in order to store recorded audio coming from
	Mixer. */

	std::function<std::unique_ptr<Wave>(Frame)> onChannelRecorded;

	/* onChannelPlayStatusChanged
	Fired when the play status of a Sample channel has changed. */

	std::function<void(ID, ChannelStatus)> onChannelPlayStatusChanged;

private:
	void loadSampleChannel(Channel&, Wave*, std::size_t scene) const;

	/* setupChannelCallbacks
	Prepares the channel with the necessary callbacks. Call this whenever a
	new channel is created. */

	void setupChannelCallbacks(const Channel&, ChannelShared&) const;

	std::vector<Channel*> getRecordableChannels(std::size_t scene);
	std::vector<Channel*> getOverdubbableChannels(std::size_t scene);

	/* setupChannelPostRecording
	Fnialize the Sample channel after an audio recording session. */

	void setupChannelPostRecording(Channel&, Frame currentFrame);

	/* recordChannel
	Records the current Mixer audio input data into an empty channel. */

	void recordChannel(Channel&, const mcl::AudioBuffer&, Frame recordedFrames, Frame currentFrame, std::size_t scene);

	/* overdubChannel
	Records the current Mixer audio input data into a channel with an existing
	Wave, overdub mode. */

	void overdubChannel(Channel&, const mcl::AudioBuffer&, Frame currentFrame, std::size_t scene);

	void triggerOnChannelsAltered();

	model::Model&           m_model;
	KernelMidi&             m_kernelMidi;
	ActionRecorder&         m_actionRecorder;
	MidiMapper<KernelMidi>& m_midiMapper;
};
} // namespace giada::m

#endif
