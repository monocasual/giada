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

#ifndef G_CHANNEL_MANAGER_H
#define G_CHANNEL_MANAGER_H

#include "core/types.h"
#include <functional>
#include <map>
#include <memory>

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
class ChannelFactory;
class WaveFactory;
class Wave;
class Plugin;
class ChannelManager final
{
public:
	ChannelManager(model::Model&, ChannelFactory&, WaveFactory&);

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

	/* forAnyChannel
    Returns true if any channel satisfies the callback 'f'. */

	bool forAnyChannel(std::function<bool(const Channel&)> f) const;

	/* reset
	Brings channels configuration back to the initial state: two I/O master
    channels, one preview channel. */

	void reset(Frame framesInBuffer);

	/* addChannel
    Adds a new channel to the stack. */

	Channel& addChannel(ChannelType, ID columnId, int position, int bufferSize);

	/* loadSampleChannel
    Loads a new Wave inside a Sample Channel. */

	void loadSampleChannel(ID channelId, std::unique_ptr<Wave>);

	/* addAndLoadChannel
    Adds a new Sample channel into the stack and fills it with a Wave. */

	void addAndLoadSampleChannel(int bufferSize, std::unique_ptr<Wave>, ID columnId, int position);

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

	/* getColumn
	Returns all channels that belongs to column 'columnId'. Read-only. */

	std::vector<const Channel*> getColumn(ID columnId) const;

	std::vector<Channel*> getChannelsIf(std::function<bool(const Channel&)> f);
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

	model::Model&   m_model;
	ChannelFactory& m_channelFactory;
	WaveFactory&    m_waveManager;
};
} // namespace giada::m

#endif
