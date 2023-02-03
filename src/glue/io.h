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

#ifndef G_GLUE_IO_H
#define G_GLUE_IO_H

#include "core/midiEvent.h"
#include "core/model/model.h"
#include "core/types.h"

/* giada::c::io
Functions to interact with the MIDI I/O. Only the main thread can use these! */

namespace giada::m
{
class Channel;
}

namespace giada::c::io
{
struct PluginParamData
{
	int         index;
	std::string name;
	uint32_t    value;
};

struct PluginData
{
	ID                           id;
	std::string                  name;
	std::vector<PluginParamData> params;
};

struct Channel_InputData
{
	Channel_InputData() = default;
	Channel_InputData(const m::Channel&);

	ID          channelId;
	ChannelType channelType;
	bool        enabled;
	bool        velocityAsVol;
	int         filter;

	uint32_t keyPress;
	uint32_t keyRelease;
	uint32_t kill;
	uint32_t arm;
	uint32_t volume;
	uint32_t mute;
	uint32_t solo;
	uint32_t pitch;
	uint32_t readActions;

	std::vector<PluginData> plugins;
};

struct Master_InputData
{
	Master_InputData() = default;
	Master_InputData(const m::model::MidiIn&);

	bool enabled;
	int  filter;

	uint32_t rewind;
	uint32_t startStop;
	uint32_t actionRec;
	uint32_t inputRec;
	uint32_t volumeIn;
	uint32_t volumeOut;
	uint32_t beatDouble;
	uint32_t beatHalf;
	uint32_t metronome;
};

struct MidiChannel_OutputData
{
	MidiChannel_OutputData(const m::MidiSender&);

	bool enabled;
	int  filter;
};

struct Channel_OutputData
{
	Channel_OutputData() = default;
	Channel_OutputData(const m::Channel&);

	ID       channelId;
	bool     lightningEnabled;
	uint32_t lightningPlaying;
	uint32_t lightningMute;
	uint32_t lightningSolo;

	std::optional<MidiChannel_OutputData> output;
};

Channel_InputData  channel_getInputData(ID channelId);
Channel_OutputData channel_getOutputData(ID channelId);
Master_InputData   master_getInputData();

/* Channel functions. */

void channel_enableMidiLearn(ID channelId, bool v);
void channel_enableMidiLightning(ID channelId, bool v);
void channel_enableMidiOutput(ID channelId, bool v);
void channel_enableVelocityAsVol(ID channelId, bool v);
void channel_setMidiInputFilter(ID channelId, int c);
void channel_setMidiOutputFilter(ID channelId, int c);

/* channel_setKey
Set key 'k' to Sample Channel 'channelId'. Used for keyboard bindings. Returns
false if the key is not valid (because used for global bindings). */

bool channel_setKey(ID channelId, int k);

/* MIDI Learning functions. */

void channel_startMidiLearn(int param, ID channelId);
void channel_clearMidiLearn(int param, ID channelId);
void master_clearMidiLearn(int param);
void master_startMidiLearn(int param);
void stopMidiLearn();
void plugin_startMidiLearn(int paramIndex, ID pluginId);
void plugin_clearMidiLearn(int param, ID pluginId);

/* Master functions. */

void master_enableMidiLearn(bool v);
void master_setMidiFilter(int c);
} // namespace giada::c::io

#endif
