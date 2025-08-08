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

#include "src/core/patchFactory.h"
#include "src/core/const.h"
#include "src/core/mixer.h"
#include "src/gui/const.h"
#include "src/utils/fs.h"
#include "src/utils/log.h"
#include <fstream>
#include <nlohmann/json.hpp>

namespace giada::m::patchFactory
{
namespace
{
constexpr auto PATCH_KEY_HEADER                       = "header";
constexpr auto PATCH_KEY_VERSION_MAJOR                = "version_major";
constexpr auto PATCH_KEY_VERSION_MINOR                = "version_minor";
constexpr auto PATCH_KEY_VERSION_PATCH                = "version_patch";
constexpr auto PATCH_KEY_NAME                         = "name";
constexpr auto PATCH_KEY_BPM                          = "bpm";
constexpr auto PATCH_KEY_BARS                         = "bars";
constexpr auto PATCH_KEY_BEATS                        = "beats";
constexpr auto PATCH_KEY_QUANTIZE                     = "quantize";
constexpr auto PATCH_KEY_METRONOME                    = "metronome";
constexpr auto PATCH_KEY_SAMPLERATE                   = "samplerate";
constexpr auto PATCH_KEY_TRACKS                       = "tracks";
constexpr auto PATCH_KEY_PLUGINS                      = "plugins";
constexpr auto PATCH_KEY_CHANNELS                     = "channels";
constexpr auto PATCH_KEY_CHANNEL_TYPE                 = "type";
constexpr auto PATCH_KEY_CHANNEL_ID                   = "id";
constexpr auto PATCH_KEY_CHANNEL_SIZE                 = "size";
constexpr auto PATCH_KEY_CHANNEL_NAME                 = "name";
constexpr auto PATCH_KEY_CHANNEL_MUTE                 = "mute";
constexpr auto PATCH_KEY_CHANNEL_SOLO                 = "solo";
constexpr auto PATCH_KEY_CHANNEL_VOLUME               = "volume";
constexpr auto PATCH_KEY_CHANNEL_PAN                  = "pan";
constexpr auto PATCH_KEY_CHANNEL_SEND_TO_MASTER       = "send_to_master";
constexpr auto PATCH_KEY_CHANNEL_EXTRA_OUTPUTS        = "extra_outputs";
constexpr auto PATCH_KEY_CHANNEL_MIDI_IN              = "midi_in";
constexpr auto PATCH_KEY_CHANNEL_MIDI_IN_VELO_AS_VOL  = "midi_in_velo_as_vol";
constexpr auto PATCH_KEY_CHANNEL_MIDI_IN_KEYPRESS     = "midi_in_keypress";
constexpr auto PATCH_KEY_CHANNEL_MIDI_IN_KEYREL       = "midi_in_keyrel";
constexpr auto PATCH_KEY_CHANNEL_MIDI_IN_KILL         = "midi_in_kill";
constexpr auto PATCH_KEY_CHANNEL_MIDI_IN_ARM          = "midi_in_arm";
constexpr auto PATCH_KEY_CHANNEL_MIDI_IN_VOLUME       = "midi_in_volume";
constexpr auto PATCH_KEY_CHANNEL_MIDI_IN_MUTE         = "midi_in_mute";
constexpr auto PATCH_KEY_CHANNEL_MIDI_IN_FILTER       = "midi_in_filter";
constexpr auto PATCH_KEY_CHANNEL_MIDI_IN_SOLO         = "midi_in_solo";
constexpr auto PATCH_KEY_CHANNEL_MIDI_OUT_L           = "midi_out_l";
constexpr auto PATCH_KEY_CHANNEL_MIDI_OUT_L_PLAYING   = "midi_out_l_playing";
constexpr auto PATCH_KEY_CHANNEL_MIDI_OUT_L_MUTE      = "midi_out_l_mute";
constexpr auto PATCH_KEY_CHANNEL_MIDI_OUT_L_SOLO      = "midi_out_l_solo";
constexpr auto PATCH_KEY_CHANNEL_WAVE_ID              = "wave_id";
constexpr auto PATCH_KEY_CHANNEL_KEY                  = "key";
constexpr auto PATCH_KEY_CHANNEL_MODE                 = "mode";
constexpr auto PATCH_KEY_CHANNEL_BEGIN                = "begin";
constexpr auto PATCH_KEY_CHANNEL_END                  = "end";
constexpr auto PATCH_KEY_CHANNEL_SHIFT                = "shift";
constexpr auto PATCH_KEY_CHANNEL_HAS_ACTIONS          = "has_actions";
constexpr auto PATCH_KEY_CHANNEL_READ_ACTIONS         = "read_actions";
constexpr auto PATCH_KEY_CHANNEL_PITCH                = "pitch";
constexpr auto PATCH_KEY_CHANNEL_INPUT_MONITOR        = "input_monitor";
constexpr auto PATCH_KEY_CHANNEL_OVERDUB_PROTECTION   = "overdub_protection";
constexpr auto PATCH_KEY_CHANNEL_MIDI_IN_READ_ACTIONS = "midi_in_read_actions";
constexpr auto PATCH_KEY_CHANNEL_MIDI_IN_PITCH        = "midi_in_pitch";
constexpr auto PATCH_KEY_CHANNEL_MIDI_OUT             = "midi_out";
constexpr auto PATCH_KEY_CHANNEL_MIDI_OUT_CHAN        = "midi_out_chan";
constexpr auto PATCH_KEY_CHANNEL_PLUGINS              = "plugins";
constexpr auto PATCH_KEY_CHANNEL_ARMED                = "armed";
constexpr auto PATCH_KEY_WAVES                        = "waves";
constexpr auto PATCH_KEY_WAVE_ID                      = "id";
constexpr auto PATCH_KEY_WAVE_PATH                    = "path";
constexpr auto PATCH_KEY_ACTIONS                      = "actions";
constexpr auto PATCH_KEY_PLUGIN_ID                    = "id";
constexpr auto PATCH_KEY_PLUGIN_JUCE_ID               = "juce_id";
constexpr auto PATCH_KEY_PLUGIN_JUCE_ID_deprecated    = "path";
constexpr auto PATCH_KEY_PLUGIN_BYPASS                = "bypass";
constexpr auto PATCH_KEY_PLUGIN_STATE                 = "state";
constexpr auto PATCH_KEY_PLUGIN_MIDI_IN_PARAMS        = "midi_in_params";
constexpr auto PATCH_KEY_TRACK_WIDTH                  = "width";
constexpr auto PATCH_KEY_TRACK_INTERNAL               = "internal";
constexpr auto PATCH_KEY_TRACK_CHANNELS               = "channels";
constexpr auto G_PATCH_KEY_ACTION_ID                  = "id";
constexpr auto G_PATCH_KEY_ACTION_CHANNEL             = "channel";
constexpr auto G_PATCH_KEY_ACTION_FRAME               = "frame";
constexpr auto G_PATCH_KEY_ACTION_EVENT               = "event";
constexpr auto G_PATCH_KEY_ACTION_PREV                = "prev";
constexpr auto G_PATCH_KEY_ACTION_NEXT                = "next";

/* -------------------------------------------------------------------------- */

void readCommons_(Patch& patch, const nlohmann::json& j)
{
	patch.name       = j.value(PATCH_KEY_NAME, G_DEFAULT_PATCH_NAME);
	patch.bars       = j.value(PATCH_KEY_BARS, G_DEFAULT_BARS);
	patch.beats      = j.value(PATCH_KEY_BEATS, G_DEFAULT_BEATS);
	patch.bpm        = j.value(PATCH_KEY_BPM, G_DEFAULT_BPM);
	patch.quantize   = j.value(PATCH_KEY_QUANTIZE, G_DEFAULT_QUANTIZE);
	patch.samplerate = j.value(PATCH_KEY_SAMPLERATE, G_DEFAULT_SAMPLERATE);
	patch.metronome  = j.value(PATCH_KEY_METRONOME, false);
}

/* -------------------------------------------------------------------------- */

void readTracks_(Patch& patch, const nlohmann::json& j)
{
	if (!j.contains(PATCH_KEY_TRACKS))
		return;

	for (const auto& jtrack : j[PATCH_KEY_TRACKS])
	{
		Patch::Track track;
		track.width    = jtrack.value(PATCH_KEY_TRACK_WIDTH, G_DEFAULT_TRACK_WIDTH);
		track.internal = jtrack.value(PATCH_KEY_TRACK_INTERNAL, false);
		if (jtrack.contains(PATCH_KEY_TRACK_CHANNELS))
			for (const auto& jplugin : jtrack[PATCH_KEY_TRACK_CHANNELS])
				track.channels.push_back(jplugin);

		patch.tracks.push_back(track);
	}
}

/* -------------------------------------------------------------------------- */

void readPlugins_(Patch& patch, const nlohmann::json& j)
{
	if (!j.contains(PATCH_KEY_PLUGINS))
		return;

	ID id = 0;
	for (const auto& jplugin : j[PATCH_KEY_PLUGINS])
	{
		Patch::Plugin p;
		p.id     = jplugin.value(PATCH_KEY_PLUGIN_ID, ++id);
		p.juceId = jplugin.value(PATCH_KEY_PLUGIN_JUCE_ID, "");
		p.bypass = jplugin.value(PATCH_KEY_PLUGIN_BYPASS, false);
		p.state  = jplugin.value(PATCH_KEY_PLUGIN_STATE, "");
		/* Patches < 1.3.0 have the deprecated JUCE id for plug-ins. */
		if (patch.version < Version{1, 3, 0})
			p.juceId = jplugin.value(PATCH_KEY_PLUGIN_JUCE_ID_deprecated, "");

		for (const auto& jmidiParam : jplugin[PATCH_KEY_PLUGIN_MIDI_IN_PARAMS])
			p.midiInParams.push_back(jmidiParam);

		patch.plugins.push_back(p);
	}
}

/* -------------------------------------------------------------------------- */

void readWaves_(Patch& patch, const nlohmann::json& j, const std::string& basePath)
{
	if (!j.contains(PATCH_KEY_WAVES))
		return;

	ID id = 0;
	for (const auto& jwave : j[PATCH_KEY_WAVES])
	{
		Patch::Wave w;
		w.id   = jwave.value(PATCH_KEY_WAVE_ID, ++id);
		w.path = u::fs::join(basePath, jwave.value(PATCH_KEY_WAVE_PATH, ""));
		patch.waves.push_back(w);
	}
}

/* -------------------------------------------------------------------------- */

void readActions_(Patch& patch, const nlohmann::json& j)
{
	if (!j.contains(PATCH_KEY_ACTIONS))
		return;

	ID id = 0;
	for (const auto& jaction : j[PATCH_KEY_ACTIONS])
	{
		Patch::Action a;
		a.id        = jaction.value(G_PATCH_KEY_ACTION_ID, ++id);
		a.channelId = jaction.value(G_PATCH_KEY_ACTION_CHANNEL, 0);
		a.frame     = jaction.value(G_PATCH_KEY_ACTION_FRAME, 0);
		a.event     = jaction.value(G_PATCH_KEY_ACTION_EVENT, 0);
		a.prevId    = jaction.value(G_PATCH_KEY_ACTION_PREV, 0);
		a.nextId    = jaction.value(G_PATCH_KEY_ACTION_NEXT, 0);
		patch.actions.push_back(a);
	}
}

/* -------------------------------------------------------------------------- */

void readChannels_(Patch& patch, const nlohmann::json& j)
{
	if (!j.contains(PATCH_KEY_CHANNELS))
		return;

	ID defaultId = PREVIEW_CHANNEL_ID;

	for (const auto& jchannel : j[PATCH_KEY_CHANNELS])
	{
		Patch::Channel c;
		c.id                = jchannel.value(PATCH_KEY_CHANNEL_ID, ++defaultId);
		c.type              = static_cast<ChannelType>(jchannel.value(PATCH_KEY_CHANNEL_TYPE, 1));
		c.volume            = jchannel.value(PATCH_KEY_CHANNEL_VOLUME, G_DEFAULT_VOL);
		c.height            = jchannel.value(PATCH_KEY_CHANNEL_SIZE, G_GUI_UNIT);
		c.name              = jchannel.value(PATCH_KEY_CHANNEL_NAME, "");
		c.key               = jchannel.value(PATCH_KEY_CHANNEL_KEY, 0);
		c.mute              = jchannel.value(PATCH_KEY_CHANNEL_MUTE, 0);
		c.solo              = jchannel.value(PATCH_KEY_CHANNEL_SOLO, 0);
		c.pan               = jchannel.value(PATCH_KEY_CHANNEL_PAN, 0.5f);
		c.hasActions        = jchannel.value(PATCH_KEY_CHANNEL_HAS_ACTIONS, false);
		c.sendToMaster      = jchannel.value(PATCH_KEY_CHANNEL_SEND_TO_MASTER, true);
		c.midiIn            = jchannel.value(PATCH_KEY_CHANNEL_MIDI_IN, 0);
		c.midiInKeyPress    = jchannel.value(PATCH_KEY_CHANNEL_MIDI_IN_KEYPRESS, 0);
		c.midiInKeyRel      = jchannel.value(PATCH_KEY_CHANNEL_MIDI_IN_KEYREL, 0);
		c.midiInKill        = jchannel.value(PATCH_KEY_CHANNEL_MIDI_IN_KILL, 0);
		c.midiInArm         = jchannel.value(PATCH_KEY_CHANNEL_MIDI_IN_ARM, 0);
		c.midiInVolume      = jchannel.value(PATCH_KEY_CHANNEL_MIDI_IN_VOLUME, 0);
		c.midiInMute        = jchannel.value(PATCH_KEY_CHANNEL_MIDI_IN_MUTE, 0);
		c.midiInSolo        = jchannel.value(PATCH_KEY_CHANNEL_MIDI_IN_SOLO, 0);
		c.midiInFilter      = jchannel.value(PATCH_KEY_CHANNEL_MIDI_IN_FILTER, 0);
		c.midiOutL          = jchannel.value(PATCH_KEY_CHANNEL_MIDI_OUT_L, 0);
		c.midiOutLplaying   = jchannel.value(PATCH_KEY_CHANNEL_MIDI_OUT_L_PLAYING, 0);
		c.midiOutLmute      = jchannel.value(PATCH_KEY_CHANNEL_MIDI_OUT_L_MUTE, 0);
		c.midiOutLsolo      = jchannel.value(PATCH_KEY_CHANNEL_MIDI_OUT_L_SOLO, 0);
		c.armed             = jchannel.value(PATCH_KEY_CHANNEL_ARMED, false);
		c.mode              = static_cast<SamplePlayerMode>(jchannel.value(PATCH_KEY_CHANNEL_MODE, 1));
		c.waveId            = jchannel.value(PATCH_KEY_CHANNEL_WAVE_ID, 0);
		c.begin             = jchannel.value(PATCH_KEY_CHANNEL_BEGIN, 0);
		c.end               = jchannel.value(PATCH_KEY_CHANNEL_END, 0);
		c.shift             = jchannel.value(PATCH_KEY_CHANNEL_SHIFT, 0);
		c.readActions       = jchannel.value(PATCH_KEY_CHANNEL_READ_ACTIONS, false);
		c.pitch             = jchannel.value(PATCH_KEY_CHANNEL_PITCH, G_DEFAULT_PITCH);
		c.inputMonitor      = jchannel.value(PATCH_KEY_CHANNEL_INPUT_MONITOR, false);
		c.overdubProtection = jchannel.value(PATCH_KEY_CHANNEL_OVERDUB_PROTECTION, false);
		c.midiInVeloAsVol   = jchannel.value(PATCH_KEY_CHANNEL_MIDI_IN_VELO_AS_VOL, 0);
		c.midiInReadActions = jchannel.value(PATCH_KEY_CHANNEL_MIDI_IN_READ_ACTIONS, 0);
		c.midiInPitch       = jchannel.value(PATCH_KEY_CHANNEL_MIDI_IN_PITCH, 0);
		c.midiOut           = jchannel.value(PATCH_KEY_CHANNEL_MIDI_OUT, 0);
		c.midiOutChan       = jchannel.value(PATCH_KEY_CHANNEL_MIDI_OUT_CHAN, 0);

		if (jchannel.contains(PATCH_KEY_CHANNEL_PLUGINS))
			for (const auto& jplugin : jchannel[PATCH_KEY_CHANNEL_PLUGINS])
				c.pluginIds.push_back(jplugin);

		if (jchannel.contains(PATCH_KEY_CHANNEL_EXTRA_OUTPUTS))
			for (const auto& joutput : jchannel[PATCH_KEY_CHANNEL_EXTRA_OUTPUTS])
				c.extraOutputs.push_back(joutput);

		patch.channels.push_back(c);
	}
}

/* -------------------------------------------------------------------------- */

void writePlugins_(const Patch& patch, nlohmann::json& j)
{
	j[PATCH_KEY_PLUGINS] = nlohmann::json::array();

	for (const Patch::Plugin& p : patch.plugins)
	{
		nlohmann::json jplugin;

		jplugin[PATCH_KEY_PLUGIN_ID]      = p.id;
		jplugin[PATCH_KEY_PLUGIN_JUCE_ID] = p.juceId;
		jplugin[PATCH_KEY_PLUGIN_BYPASS]  = p.bypass;
		jplugin[PATCH_KEY_PLUGIN_STATE]   = p.state;

		jplugin[PATCH_KEY_PLUGIN_MIDI_IN_PARAMS] = nlohmann::json::array();
		for (uint32_t param : p.midiInParams)
			jplugin[PATCH_KEY_PLUGIN_MIDI_IN_PARAMS].push_back(param);

		j[PATCH_KEY_PLUGINS].push_back(jplugin);
	}
}

/* -------------------------------------------------------------------------- */

void writeTracks_(const Patch& patch, nlohmann::json& j)
{
	j[PATCH_KEY_TRACKS] = nlohmann::json::array();

	for (const Patch::Track& track : patch.tracks)
	{
		nlohmann::json jtrack;
		jtrack[PATCH_KEY_TRACK_WIDTH]    = track.width;
		jtrack[PATCH_KEY_TRACK_INTERNAL] = track.internal;
		jtrack[PATCH_KEY_TRACK_CHANNELS] = nlohmann::json::array();
		for (ID channelId : track.channels)
			jtrack[PATCH_KEY_TRACK_CHANNELS].push_back(channelId);

		j[PATCH_KEY_TRACKS].push_back(jtrack);
	}
}

/* -------------------------------------------------------------------------- */

void writeActions_(const Patch& patch, nlohmann::json& j)
{
	j[PATCH_KEY_ACTIONS] = nlohmann::json::array();

	for (const Patch::Action& a : patch.actions)
	{
		nlohmann::json jaction;
		jaction[G_PATCH_KEY_ACTION_ID]      = a.id;
		jaction[G_PATCH_KEY_ACTION_CHANNEL] = a.channelId;
		jaction[G_PATCH_KEY_ACTION_FRAME]   = a.frame;
		jaction[G_PATCH_KEY_ACTION_EVENT]   = a.event;
		jaction[G_PATCH_KEY_ACTION_PREV]    = a.prevId;
		jaction[G_PATCH_KEY_ACTION_NEXT]    = a.nextId;
		j[PATCH_KEY_ACTIONS].push_back(jaction);
	}
}

/* -------------------------------------------------------------------------- */

void writeWaves_(const Patch& patch, nlohmann::json& j)
{
	j[PATCH_KEY_WAVES] = nlohmann::json::array();

	for (const Patch::Wave& w : patch.waves)
	{
		nlohmann::json jwave;
		jwave[PATCH_KEY_WAVE_ID]   = w.id;
		jwave[PATCH_KEY_WAVE_PATH] = w.path;

		j[PATCH_KEY_WAVES].push_back(jwave);
	}
}

/* -------------------------------------------------------------------------- */

void writeCommons_(const Patch& patch, nlohmann::json& j)
{
	j[PATCH_KEY_HEADER]        = "GIADAPTC";
	j[PATCH_KEY_VERSION_MAJOR] = G_VERSION.getMajor();
	j[PATCH_KEY_VERSION_MINOR] = G_VERSION.getMinor();
	j[PATCH_KEY_VERSION_PATCH] = G_VERSION.getPatch();
	j[PATCH_KEY_NAME]          = patch.name;
	j[PATCH_KEY_BARS]          = patch.bars;
	j[PATCH_KEY_BEATS]         = patch.beats;
	j[PATCH_KEY_BPM]           = patch.bpm;
	j[PATCH_KEY_QUANTIZE]      = patch.quantize;
	j[PATCH_KEY_SAMPLERATE]    = patch.samplerate;
	j[PATCH_KEY_METRONOME]     = patch.metronome;
}

/* -------------------------------------------------------------------------- */

void writeChannels_(const Patch& patch, nlohmann::json& j)
{
	j[PATCH_KEY_CHANNELS] = nlohmann::json::array();

	for (const Patch::Channel& c : patch.channels)
	{
		nlohmann::json jchannel;

		jchannel[PATCH_KEY_CHANNEL_ID]                   = c.id;
		jchannel[PATCH_KEY_CHANNEL_TYPE]                 = static_cast<int>(c.type);
		jchannel[PATCH_KEY_CHANNEL_SIZE]                 = c.height;
		jchannel[PATCH_KEY_CHANNEL_NAME]                 = c.name;
		jchannel[PATCH_KEY_CHANNEL_MUTE]                 = c.mute;
		jchannel[PATCH_KEY_CHANNEL_SOLO]                 = c.solo;
		jchannel[PATCH_KEY_CHANNEL_VOLUME]               = c.volume;
		jchannel[PATCH_KEY_CHANNEL_PAN]                  = c.pan;
		jchannel[PATCH_KEY_CHANNEL_HAS_ACTIONS]          = c.hasActions;
		jchannel[PATCH_KEY_CHANNEL_ARMED]                = c.armed;
		jchannel[PATCH_KEY_CHANNEL_SEND_TO_MASTER]       = c.sendToMaster;
		jchannel[PATCH_KEY_CHANNEL_MIDI_IN]              = c.midiIn;
		jchannel[PATCH_KEY_CHANNEL_MIDI_IN_KEYREL]       = c.midiInKeyRel;
		jchannel[PATCH_KEY_CHANNEL_MIDI_IN_KEYPRESS]     = c.midiInKeyPress;
		jchannel[PATCH_KEY_CHANNEL_MIDI_IN_KILL]         = c.midiInKill;
		jchannel[PATCH_KEY_CHANNEL_MIDI_IN_ARM]          = c.midiInArm;
		jchannel[PATCH_KEY_CHANNEL_MIDI_IN_VOLUME]       = c.midiInVolume;
		jchannel[PATCH_KEY_CHANNEL_MIDI_IN_MUTE]         = c.midiInMute;
		jchannel[PATCH_KEY_CHANNEL_MIDI_IN_SOLO]         = c.midiInSolo;
		jchannel[PATCH_KEY_CHANNEL_MIDI_IN_FILTER]       = c.midiInFilter;
		jchannel[PATCH_KEY_CHANNEL_MIDI_OUT_L]           = c.midiOutL;
		jchannel[PATCH_KEY_CHANNEL_MIDI_OUT_L_PLAYING]   = c.midiOutLplaying;
		jchannel[PATCH_KEY_CHANNEL_MIDI_OUT_L_MUTE]      = c.midiOutLmute;
		jchannel[PATCH_KEY_CHANNEL_MIDI_OUT_L_SOLO]      = c.midiOutLsolo;
		jchannel[PATCH_KEY_CHANNEL_KEY]                  = c.key;
		jchannel[PATCH_KEY_CHANNEL_WAVE_ID]              = c.waveId;
		jchannel[PATCH_KEY_CHANNEL_MODE]                 = static_cast<int>(c.mode);
		jchannel[PATCH_KEY_CHANNEL_BEGIN]                = c.begin;
		jchannel[PATCH_KEY_CHANNEL_END]                  = c.end;
		jchannel[PATCH_KEY_CHANNEL_SHIFT]                = c.shift;
		jchannel[PATCH_KEY_CHANNEL_READ_ACTIONS]         = c.readActions;
		jchannel[PATCH_KEY_CHANNEL_PITCH]                = c.pitch;
		jchannel[PATCH_KEY_CHANNEL_INPUT_MONITOR]        = c.inputMonitor;
		jchannel[PATCH_KEY_CHANNEL_OVERDUB_PROTECTION]   = c.overdubProtection;
		jchannel[PATCH_KEY_CHANNEL_MIDI_IN_VELO_AS_VOL]  = c.midiInVeloAsVol;
		jchannel[PATCH_KEY_CHANNEL_MIDI_IN_READ_ACTIONS] = c.midiInReadActions;
		jchannel[PATCH_KEY_CHANNEL_MIDI_IN_PITCH]        = c.midiInPitch;
		jchannel[PATCH_KEY_CHANNEL_MIDI_OUT]             = c.midiOut;
		jchannel[PATCH_KEY_CHANNEL_MIDI_OUT_CHAN]        = c.midiOutChan;

		jchannel[PATCH_KEY_CHANNEL_PLUGINS] = nlohmann::json::array();
		for (ID pid : c.pluginIds)
			jchannel[PATCH_KEY_CHANNEL_PLUGINS].push_back(pid);

		jchannel[PATCH_KEY_CHANNEL_EXTRA_OUTPUTS] = nlohmann::json::array();
		for (int output : c.extraOutputs)
			jchannel[PATCH_KEY_CHANNEL_EXTRA_OUTPUTS].push_back(output);

		j[PATCH_KEY_CHANNELS].push_back(jchannel);
	}
}

/* -------------------------------------------------------------------------- */

void modernize_(Patch& patch)
{
	/* 1.1.0
	Older patches don't contain Tracks. Let's recreate a basic Track layout: one
	internal, one external. */
	if (patch.version < Version{1, 1, 0})
	{
		const ID groupChannelId = 8192;
		patch.tracks.push_back({G_DEFAULT_TRACK_WIDTH, /*internal=*/true, {}});
		patch.tracks.push_back({G_DEFAULT_TRACK_WIDTH, /*internal=*/false, {groupChannelId}});
		patch.channels.push_back({groupChannelId, ChannelType::GROUP, G_GUI_UNIT});
	}

	for (Patch::Channel& c : patch.channels)
	{
		const bool isInternalChannel = c.type == ChannelType::PREVIEW || c.type == ChannelType::MASTER;

		/* 0.16.3
		Make sure that ChannelType is correct: ID 1, 2 are MASTER channels, ID 3
		is PREVIEW channel. */
		if (c.id == MASTER_OUT_CHANNEL_ID || c.id == MASTER_IN_CHANNEL_ID)
			c.type = ChannelType::MASTER;
		else if (c.id == PREVIEW_CHANNEL_ID)
			c.type = ChannelType::PREVIEW;

		/* 0.16.4
		Make sure internal channels are never armed. */
		if (isInternalChannel)
			c.armed = false;

		/* 0.16.3
		Set panning to default (0.5) and waveId to 0 for non-Sample Channels. */
		if (c.type != ChannelType::SAMPLE)
		{
			c.pan    = G_DEFAULT_PAN;
			c.waveId = 0;
		}

		/* 1.1.0
		Let's put non-group channels into the relevant tracks. */
		if (patch.version < Version{1, 1, 0} && c.type != ChannelType::GROUP)
		{
			const std::size_t targetIndex = isInternalChannel ? 0 : 1;
			patch.tracks[targetIndex].channels.push_back(c.id);
		}
	}
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

bool serialize(const Patch& patch, const std::string& filePath)
{
	nlohmann::json j;

	writeCommons_(patch, j);
	writeTracks_(patch, j);
	writeChannels_(patch, j);
	writeActions_(patch, j);
	writeWaves_(patch, j);
	writePlugins_(patch, j);

	std::ofstream ofs(filePath);
	if (!ofs.good())
		return false;

	ofs << j;
	return true;
}

/* -------------------------------------------------------------------------- */

Patch deserialize(const std::string& filePath)
{
	Patch patch;

	std::ifstream ifs(filePath);
	if (!ifs.good())
	{
		patch.status = G_FILE_UNREADABLE;
		return patch;
	}

	try
	{
		nlohmann::json j = nlohmann::json::parse(ifs);

		if (j[PATCH_KEY_HEADER] != "GIADAPTC")
		{
			patch.status = G_FILE_INVALID;
			return patch;
		}

		patch.version = {
		    static_cast<int>(j[PATCH_KEY_VERSION_MAJOR]),
		    static_cast<int>(j[PATCH_KEY_VERSION_MINOR]),
		    static_cast<int>(j[PATCH_KEY_VERSION_PATCH])};
		if (patch.version < Version{0, 16, 0})
		{
			patch.status = G_FILE_UNSUPPORTED;
			return patch;
		}

		readCommons_(patch, j);
		readTracks_(patch, j);
		readPlugins_(patch, j);
		readWaves_(patch, j, u::fs::dirname(filePath));
		readActions_(patch, j);
		readChannels_(patch, j);
		modernize_(patch);
	}
	catch (nlohmann::json::exception& e)
	{
		u::log::print("[patchFactory::deserialize] Exception thrown: {}\n", e.what());
		patch.status = G_FILE_INVALID;
		return patch;
	}

	patch.status = G_FILE_OK;
	return patch;
}
} // namespace giada::m::patchFactory