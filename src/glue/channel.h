	/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#include <optional>
#include <atomic>
#include <string>
#include <vector>
#include "core/model/model.h"
#include "core/types.h"


namespace giada {
namespace m
{
class Channel;
class SamplePlayer;
}
namespace c {
namespace channel 
{
struct SampleData
{
	SampleData() = delete;
	SampleData(const m::SamplePlayer&, const m::AudioReceiver&);

	Frame a_getTracker() const;
	Frame a_getBegin() const;
	Frame a_getEnd() const;
	bool  a_getInputMonitor() const;
	bool  a_getOverdubProtection() const;

	ID               waveId;
	SamplePlayerMode mode;
	bool             isLoop;
	float            pitch;

private:

	const m::SamplePlayer*  m_samplePlayer;
	const m::AudioReceiver* m_audioReceiver;
};

struct MidiData
{
	MidiData() = delete;
	MidiData(const m::MidiSender&);

	bool a_isOutputEnabled() const;
	int  a_getFilter() const;

private:

	const m::MidiSender* m_midiSender;
};

struct Data
{
	Data() = default;
	Data(const m::Channel&);

	bool a_getMute() const;
	bool a_getSolo() const;
	ChannelStatus a_getPlayStatus() const;
	ChannelStatus a_getRecStatus() const;
	bool a_getReadActions() const;
	bool a_isArmed() const;
	bool a_isRecordingInput() const;
	bool a_isRecordingAction() const;

	ID              id;
	ID              columnId;
#ifdef WITH_VST
	std::vector<ID> pluginIds;
#endif
	ChannelType     type;
	Pixel           height;
	std::string     name;
	float           volume;
	float           pan;
	int             key;
	bool            hasActions;

	std::optional<SampleData> sample;
	std::optional<MidiData>   midi;

private:

	const m::Channel& m_channel;
};

/* getChannels
Returns a single viewModel object filled with data from a channel. */

Data getData(ID channelId);

/* getChannels
Returns a vector of viewModel objects filled with data from channels. */

std::vector<Data> getChannels();

/* a_get
Returns an atomic property from a Channel, by locking it first. */

template <typename T>
T a_get(const std::atomic<T>& a)
{
	m::model::ChannelsLock l(m::model::channels);
	return a.load();
}

/* addChannel
Adds an empty new channel to the stack. */

void addChannel(ID columnId, ChannelType type);

/* loadChannel
Fills an existing channel with a wave. */

int loadChannel(ID columnId, const std::string& fname);

/* addAndLoadChannel
Adds a new Sample Channel and fills it with a wave right away. */

void addAndLoadChannel(ID columnId, const std::string& fpath); 

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

/* set*
Sets several channel properties. */

void setInputMonitor(ID channelId, bool value);
void setOverdubProtection(ID channelId, bool value);
void setName(ID channelId, const std::string& name);
void setHeight(ID channelId, Pixel p);

void setSamplePlayerMode(ID channelId, SamplePlayerMode m);
}}}; // giada::c::channel::

#endif
