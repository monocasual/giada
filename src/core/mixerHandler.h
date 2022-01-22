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

#ifndef G_MIXER_HANDLER_H
#define G_MIXER_HANDLER_H

#include "core/plugins/pluginManager.h"
#include "core/waveManager.h"
#include "types.h"
#include <functional>
#include <memory>
#include <string>

namespace giada::m::model
{
class Model;
}

namespace giada::m
{
class Channel;
class Wave;
class Mixer;
class Plugin;
class ChannelManager;
class Sequencer;
class MixerHandler final
{
public:
	MixerHandler(model::Model&, Mixer&);

	/* hasLogicalSamples
    True if 1 or more samples are logical (memory only, such as takes). */

	bool hasLogicalSamples() const;

	/* hasEditedSamples
    True if 1 or more samples have been edited via Sample Editor. */

	bool hasEditedSamples() const;

	/* has(Input|Action)RecordableChannels
    Tells whether Mixer has one or more input or action recordable channels. */

	bool hasInputRecordableChannels() const;
	bool hasActionRecordableChannels() const;

	/* hasActions
    True if at least one Channel has actions recorded in it. */

	bool hasActions() const;

	/* hasAudioData
    True if at least one Sample Channel has some audio recorded in it. */

	bool hasAudioData() const;

	float getInVol() const;
	float getOutVol() const;
	bool  getInToOut() const;

	/* reset
	Brings everything back to the initial state. */

	void reset(Frame framesInLoop, Frame framesInBuffer, ChannelManager&);

	/* addChannel
    Adds a new channel of type 'type' into the channels stack. Returns the new
    channel ID. */

	Channel& addChannel(ChannelType type, ID columnId, int bufferSize, ChannelManager&);

	/* loadChannel
    Loads a new Wave inside a Sample Channel. */

	void loadChannel(ID channelId, std::unique_ptr<Wave> w);

	/* addAndLoadChannel
    Creates a new channels, fills it with a Wave and then add it to the stack. */

	void addAndLoadChannel(ID columnId, std::unique_ptr<Wave> w, int bufferSize,
	    ChannelManager&);

	/* freeChannel
    Unloads existing Wave from a Sample Channel. */

	void freeChannel(ID channelId);

	/* deleteChannel
    Completely removes a channel from the stack. */

	void deleteChannel(ID channelId);

#ifdef WITH_VST
	void cloneChannel(ID channelId, int sampleRate, int bufferSize, ChannelManager&,
	    WaveManager&, const Sequencer&, PluginManager&);
#else
	void cloneChannel(ID channelId, int bufferSize, ChannelManager&, WaveManager&);
#endif
	void renameChannel(ID channelId, const std::string& name);
	void freeAllChannels();

	void setInToOut(bool v);

	/* updateSoloCount
    Updates the number of solo-ed channels in mixer. */

	void updateSoloCount();

	/* startInputRec
	Initializes Mixer for input recording. */

	void startInputRec(Frame currentFrame);

	/* stopInputRec
	Terminates input recording in Mixer. Returns the number of recorded frames. 
	Call finalizeInputRec() if you really want to finish the input recording 
	operation. */

	Frame stopInputRec();

	/* finalizeInputRec
    Fills armed Sample Channels with audio data coming from an input recording
    session. */

	void finalizeInputRec(Frame recordedFrames, Frame currentFrame);

	/* onChannelsAltered
	Fired when something is done on channels (added, removed, loaded, ...). */

	std::function<void()> onChannelsAltered;

	/* onChannelRecorded
	Fired during the input recording finalization, when a new empty Wave must
	be added to each armed channel in order to store recorded audio coming from
	Mixer. */

	std::function<std::unique_ptr<Wave>(Frame)> onChannelRecorded;

private:
	bool forAnyChannel(std::function<bool(const Channel&)> f) const;

	void loadChannel(Channel&, Wave*) const;

	std::vector<Channel*> getChannelsIf(std::function<bool(const Channel&)> f);
	std::vector<Channel*> getRecordableChannels();
	std::vector<Channel*> getOverdubbableChannels();

	void setupChannelPostRecording(Channel& ch, Frame currentFrame);

	/* recordChannel
	Records the current Mixer audio input data into an empty channel. */

	void recordChannel(Channel& ch, Frame recordedFrames, Frame currentFrame);

	/* overdubChannel
	Records the current Mixer audio input data into a channel with an existing
	Wave, overdub mode. */

	void overdubChannel(Channel& ch, Frame currentFrame);

	model::Model& m_model;
	Mixer&        m_mixer;
};
} // namespace giada::m

#endif
