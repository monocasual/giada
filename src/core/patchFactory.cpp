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

#include "core/patchFactory.h"
#include "core/const.h"
#include "core/mixer.h"
#include "utils/fs.h"
#include "utils/log.h"
#include <fstream>
#include <nlohmann/json.hpp>

namespace giada::m::patchFactory
{
namespace
{
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

void readColumns_(Patch& patch, const nlohmann::json& j)
{
	for (const auto& jcol : j[PATCH_KEY_COLUMNS])
	{
		Patch::Column c;
		c.width = jcol.value(PATCH_KEY_COLUMN_WIDTH, G_DEFAULT_COLUMN_WIDTH);
		if (jcol.contains(PATCH_KEY_COLUMN_CHANNELS))
			for (const auto& jplugin : jcol[PATCH_KEY_COLUMN_CHANNELS])
				c.channels.push_back(jplugin);

		patch.columns.push_back(c);
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
		p.path   = jplugin.value(PATCH_KEY_PLUGIN_PATH, "");
		p.bypass = jplugin.value(PATCH_KEY_PLUGIN_BYPASS, false);

		if (patch.version < Patch::Version{0, 17, 0})
			for (const auto& jparam : jplugin[PATCH_KEY_PLUGIN_PARAMS])
				p.params.push_back(jparam);
		else
			p.state = jplugin.value(PATCH_KEY_PLUGIN_STATE, "");

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

	ID defaultId = Mixer::PREVIEW_CHANNEL_ID;

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

		jplugin[PATCH_KEY_PLUGIN_ID]     = p.id;
		jplugin[PATCH_KEY_PLUGIN_PATH]   = p.path;
		jplugin[PATCH_KEY_PLUGIN_BYPASS] = p.bypass;
		jplugin[PATCH_KEY_PLUGIN_STATE]  = p.state;

		jplugin[PATCH_KEY_PLUGIN_MIDI_IN_PARAMS] = nlohmann::json::array();
		for (uint32_t param : p.midiInParams)
			jplugin[PATCH_KEY_PLUGIN_MIDI_IN_PARAMS].push_back(param);

		j[PATCH_KEY_PLUGINS].push_back(jplugin);
	}
}

/* -------------------------------------------------------------------------- */

void writeColumns_(const Patch& patch, nlohmann::json& j)
{
	j[PATCH_KEY_COLUMNS] = nlohmann::json::array();

	for (const Patch::Column& column : patch.columns)
	{
		nlohmann::json jcolumn;
		jcolumn[PATCH_KEY_COLUMN_WIDTH]    = column.width;
		jcolumn[PATCH_KEY_COLUMN_CHANNELS] = nlohmann::json::array();
		for (ID channelId : column.channels)
			jcolumn[PATCH_KEY_COLUMN_CHANNELS].push_back(channelId);

		j[PATCH_KEY_COLUMNS].push_back(jcolumn);
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
	j[PATCH_KEY_VERSION_MAJOR] = G_VERSION_MAJOR;
	j[PATCH_KEY_VERSION_MINOR] = G_VERSION_MINOR;
	j[PATCH_KEY_VERSION_PATCH] = G_VERSION_PATCH;
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

		j[PATCH_KEY_CHANNELS].push_back(jchannel);
	}
}

/* -------------------------------------------------------------------------- */

void modernize_(Patch& patch)
{
	for (Patch::Channel& c : patch.channels)
	{
		const bool isInternalChannel = c.type == ChannelType::PREVIEW || c.type == ChannelType::MASTER;

		/* 0.16.3
		Make sure that ChannelType is correct: ID 1, 2 are MASTER channels, ID 3 
		is PREVIEW channel. */
		if (c.id == Mixer::MASTER_OUT_CHANNEL_ID || c.id == Mixer::MASTER_IN_CHANNEL_ID)
			c.type = ChannelType::MASTER;
		else if (c.id == Mixer::PREVIEW_CHANNEL_ID)
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

		/* 1.0.0 
		The relationship column-channel has been simplified. Let's put all
		channels in the first column as a fallback. */
		if (patch.version < Patch::Version{1, 0, 0} && !isInternalChannel)
		{
			if (patch.columns.empty())
				patch.columns.push_back({G_DEFAULT_COLUMN_WIDTH, {}});
			patch.columns[0].channels.push_back(c.id);
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
	writeColumns_(patch, j);
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
	if (patch.version < Patch::Version{0, 16, 0})
	{
		patch.status = G_FILE_UNSUPPORTED;
		return patch;
	}

	try
	{
		readCommons_(patch, j);
		readColumns_(patch, j);
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