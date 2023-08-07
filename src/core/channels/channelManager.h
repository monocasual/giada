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

#ifndef G_CHANNEL_MANAGER_H
#define G_CHANNEL_MANAGER_H

#include "core/channels/sampleReactor.h"
#include "core/midiMapper.h"
#include "core/resampler.h"
#include "core/types.h"
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

	/* getAllChannels
	Returns all channel in the model. */

	std::vector<Channel>& getAllChannels();

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

	float getMasterInVol() const;
	float getMasterOutVol() const;

	/* getLastChannelPosition
	Returns the position of the last channel located in column 'columnId'. */

	int getLastChannelPosition(ID columnId) const;

	/* reset
	Brings channels configuration back to the initial state: two I/O master
    channels, one preview channel. */

	void reset(Frame framesInBuffer);

	/* setBufferSize
	Sets a new buffer size value to all channels. Must be called only when
	mixer is disabled. */

	void setBufferSize(int);

	/* addChannel
    Adds a new channel to the stack. */

	Channel& addChannel(ChannelType, ID columnId, int position, int bufferSize);

	/* loadSampleChannel (1)
    Creates a new Wave from a file path and loads it inside a Sample Channel. */

	int loadSampleChannel(ID channelId, const std::string&, int sampleRate, Resampler::Quality);

	/* loadSampleChannel (2)
    Loads an existing Wave inside a Sample Channel. */

	void loadSampleChannel(ID channelId, Wave&);

	/* freeChannel
    Unloads existing Wave from a Sample Channel. */

	void freeSampleChannel(ID channelId);
	void freeAllSampleChannels();

	void deleteChannel(ID channelId);
	void renameChannel(ID channelId, const std::string& name);
	void moveChannel(ID channelId, ID columnId, int position);

	/* cloneChannel
	Creates a duplicate of Channel. Wants a vector of already cloned plug-ins. */

	void cloneChannel(ID channelId, int bufferSize, const std::vector<Plugin*>&);

	/* finalizeInputRec
    Fills armed Sample channel with audio data coming from an input recording
    session. */

	void finalizeInputRec(const mcl::AudioBuffer&, Frame recordedFrames, Frame currentFrame);

	void keyPress(ID channelId, int velocity, bool canRecordActions, bool canQuantize, Frame currentFrameQuantized);
	void keyRelease(ID channelId, bool canRecordActions, Frame currentFrameQuantized);
	void keyKill(ID channelId, bool canRecordActions, Frame currentFrameQuantized);
	void processMidiEvent(ID channelId, const MidiEvent&, bool canRecordActions, Frame currentFrameQuantized);
	void setInputMonitor(ID channelId, bool value);
	void setVolume(ID channelId, float value);
	void setPitch(ID channelId, float value);
	void setPan(ID channelId, float value);
	void setBeginEnd(ID channelId, Frame b, Frame e);
	void resetBeginEnd(ID channelId);
	void toggleMute(ID channelId);
	void toggleSolo(ID channelId);
	void toggleArm(ID channelId);
	void toggleReadActions(ID channelId, bool seqIsRunning);
	void killReadActions(ID channelId);
	void setOverdubProtection(ID channelId, bool value);
	void setSamplePlayerMode(ID channelId, SamplePlayerMode);
	void setHeight(ID channelId, Pixel height);
	void loadWaveInPreviewChannel(ID sourceChannelId);
	void freeWaveInPreviewChannel();
	void setPreviewTracker(Frame f);
	void stopAll();
	void rewindAll();
	bool saveSample(ID channelId, const std::string& filePath);

	/* consolidateChannels
	Enable reading actions for Channels that have just been filled with actions
	after an action recording session. This will start reading actions right 
	away, without checking whether conf::treatRecsAsLoops is enabled or not.
	Same thing for MIDI channels.  */

	void consolidateChannels(const std::unordered_set<ID>&);

	/* onChannelsAltered
	Fired when something is done on channels (added, removed, loaded, ...). */

	std::function<void()> onChannelsAltered;

	/* onChannelRecorded
	Fired during the input recording finalization, when a new empty Wave must
	be added to each armed channel in order to store recorded audio coming from
	Mixer. */

	std::function<std::unique_ptr<Wave>(Frame)> onChannelRecorded;

private:
	void loadSampleChannel(Channel&, Wave*, Frame begin = -1, Frame end = -1, Frame shift = -1) const;

	/* setupChannelCallbacks
    Prepares the channel with the necessary callbacks. Call this whenever a 
	new channel is created. */

	void setupChannelCallbacks(const Channel&, ChannelShared&) const;

	std::vector<Channel*> getRecordableChannels();
	std::vector<Channel*> getOverdubbableChannels();

	/* setupChannelPostRecording
    Fnialize the Sample channel after an audio recording session. */

	void setupChannelPostRecording(Channel&, Frame currentFrame);

	/* recordChannel
	Records the current Mixer audio input data into an empty channel. */

	void recordChannel(Channel&, const mcl::AudioBuffer&, Frame recordedFrames, Frame currentFrame);

	/* overdubChannel
	Records the current Mixer audio input data into a channel with an existing
	Wave, overdub mode. */

	void overdubChannel(Channel&, const mcl::AudioBuffer&, Frame currentFrame);

	void triggerOnChannelsAltered();

	model::Model&           m_model;
	KernelMidi&             m_kernelMidi;
	ActionRecorder&         m_actionRecorder;
	MidiMapper<KernelMidi>& m_midiMapper;
	SampleReactor           m_sampleReactor;
};
} // namespace giada::m

#endif
