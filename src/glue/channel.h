/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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
Functions to interact with channels. */

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
	Data(const m::Channel&, int columnIndex, int position);

	ChannelStatus getPlayStatus() const;
	ChannelStatus getRecStatus() const;
	bool          getReadActions() const;
	bool          isRecordingInput() const;
	bool          isRecordingActions() const;
	bool          isMuted() const;
	bool          isSoloed() const;
	bool          isArmed() const;

	ID                      id;
	int                     columnIndex;
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

struct Column
{
	int               index;
	int               width;
	std::vector<Data> channels;
};

/* getChannels
Returns a single viewModel object filled with data from a channel. */

Data getData(ID channelId);

/* getColumns
Returns a vector of Column's filled with Data objects, which reflects the layout
described by Model::columns. */

std::vector<Column> getColumns();

/* addChannel
Adds an empty new channel to the stack. */

void addChannel(int columnIndex, ChannelType type);

/* loadChannel
Fills an existing channel with a wave. */

void loadChannel(int columnIndex, const std::string& fname);

/* addAndLoadChannels
As above, with multiple audio file paths in input. */

void addAndLoadChannels(int columnIndex, const std::vector<std::string>& fpaths);

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
Moves channel with channelId to column at 'columnIndex' at 'position'. */

void moveChannel(ID channelId, int columnIndex, int position);

/* addColumn
Adds a new column at the end of the list. */

void addColumn();

/* deleteColumn
Deletes column by index. */

void deleteColumn(int);

/* setColumnWidth
Set the width to 'w' pixel of column ad index 'index'. */

void setColumnWidth(int index, int w);

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

void  pressChannel(ID channelId, float velocity, Thread t);
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
