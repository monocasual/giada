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

#ifndef G_GLUE_CHANNEL_H
#define G_GLUE_CHANNEL_H

#include "core/model/model.h"
#include "core/types.h"
#include "core/weakAtomic.h"
#include <atomic>
#include <optional>
#include <string>
#include <vector>

/* giada::c::channel
Functions to interact with channels. Only the main thread can use these! */

namespace giada::m
{
class Plugin;
}

namespace giada::v
{
class Dispatcher;
}

namespace giada::c::channel
{
struct SampleData
{
	SampleData() = delete;
	SampleData(const m::Channel&);

	Frame getTracker() const;

	ID               waveId;
	SamplePlayerMode mode;
	bool             isLoop;
	float            pitch;
	Frame            begin;
	Frame            end;
	bool             inputMonitor;
	bool             overdubProtection;

private:
	WeakAtomic<Frame>* m_tracker;
};

struct MidiData
{
	MidiData() = delete;
	MidiData(const m::Channel&);

	bool isOutputEnabled;
	int  filter;
};

struct Data
{
	Data(const m::Channel&);

	ChannelStatus getPlayStatus() const;
	ChannelStatus getRecStatus() const;
	bool          getReadActions() const;
	bool          isRecordingInput() const;
	bool          isRecordingActions() const;
	bool          isMuted() const;
	bool          isSoloed() const;
	bool          isArmed() const;

	ID                      id;
	ID                      columnId;
	int                     position;
	std::vector<m::Plugin*> plugins;
	ChannelType             type;
	Pixel                   height;
	std::string             name;
	float                   volume;
	float                   pan;
	int                     key;
	bool                    hasActions;

	std::optional<SampleData> sample;
	std::optional<MidiData>   midi;

private:
	WeakAtomic<ChannelStatus>* m_playStatus;
	WeakAtomic<ChannelStatus>* m_recStatus;
	WeakAtomic<bool>*          m_readActions;
};

/* getChannels
Returns a single viewModel object filled with data from a channel. */

Data getData(ID channelId);

/* getChannels
Returns a vector of viewModel objects filled with data from channels. */

std::vector<Data> getChannels();

/* addChannel
Adds an empty new channel to the stack. */

void addChannel(ID columnId, ChannelType type);

/* loadChannel
Fills an existing channel with a wave. */

void loadChannel(ID columnId, const std::string& fname);

/* addAndLoadChannels
As above, with multiple audio file paths in input. */

void addAndLoadChannels(ID columnId, const std::vector<std::string>& fpaths);

/* deleteChannel
Removes a channel from Mixer. */

void deleteChannel(ID channelId);

/* freeChannel
Unloads the sample from a sample channel. */

void freeChannel(ID channelId);

/* cloneChannel
Makes an exact copy of a channel. */

void cloneChannel(ID channelId);

/* moveChannel
Moves channel with channelId to column with columnId at 'position'. */

void moveChannel(ID channelId, ID columnId, int position);

/* set*
Sets several channel properties. */

void setInputMonitor(ID channelId, bool value);
void setOverdubProtection(ID channelId, bool value);
void setName(ID channelId, const std::string& name);
void setHeight(ID channelId, Pixel p);

/* clearAllActions
Deletes all recorded actions on channel 'channelId'. */

void clearAllActions(ID channelId);

void setSamplePlayerMode(ID channelId, SamplePlayerMode m);

/* setCallbacks
Install callbacks to a m::Channel object in order to communicate with the UI. 
Call this whenever you add a new channel. */

void setCallbacks(m::Channel&);

void  pressChannel(ID channelId, int velocity, Thread t);
void  releaseChannel(ID channelId, Thread t);
void  killChannel(ID channelId, Thread t);
float setChannelVolume(ID channelId, float v, Thread t, bool repaintMainUi = false);
float setChannelPitch(ID channelId, float v, Thread t);
float sendChannelPan(ID channelId, float v); // FIXME typo: should be setChannelPan
void  toggleMuteChannel(ID channelId, Thread t);
void  toggleSoloChannel(ID channelId, Thread t);
void  toggleArmChannel(ID channelId, Thread t);
void  toggleReadActionsChannel(ID channelId, Thread t);
void  killReadActionsChannel(ID channelId, Thread t);
void  sendMidiToChannel(ID channelId, m::MidiEvent e, Thread t);

/* notifyChannelForMidiIn
Tells Channel with ID that a MIDI event has been received. */

void notifyChannelForMidiIn(Thread, ID channelId);
} // namespace giada::c::channel

#endif
