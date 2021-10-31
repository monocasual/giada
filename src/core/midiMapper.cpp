/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#include "core/midiMapper.h"
#include "core/const.h"
#include "core/kernelMidi.h"
#include "core/midiEvent.h"
#include "utils/fs.h"
#include "utils/log.h"
#include "utils/string.h"
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

namespace nl = nlohmann;

namespace giada::m
{
bool MidiMap::isValid() const
{
	return !(brand.empty() || device.empty());
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

MidiMapper::MidiMapper(KernelMidi& k)
: m_kernelMidi(k)
{
}

/* -------------------------------------------------------------------------- */

const MidiMap& MidiMapper::getCurrentMap() const
{
	return m_currentMap;
}

/* -------------------------------------------------------------------------- */

const std::vector<std::string>& MidiMapper::getMapFilesFound() const
{
	return m_mapFiles;
}

/* -------------------------------------------------------------------------- */

void MidiMapper::init()
{
	m_mapsPath = u::fs::getHomePath() + G_SLASH + "midimaps" + G_SLASH;

	/* scan dir of midi maps and load the filenames into m_mapFiles vector. */

	u::log::print("[MidiMapper::init] scanning midimaps directory '%s'...\n",
	    m_mapsPath);

	if (!std::filesystem::exists(m_mapsPath))
	{
		u::log::print("[MidiMapper::init] unable to scan midimaps directory!\n");
		return;
	}

	for (const auto& d : std::filesystem::directory_iterator(m_mapsPath))
	{
		// TODO - check if is a valid midiMap file (verify headers)
		if (!d.is_regular_file())
			continue;
		u::log::print("[MidiMapper::init] found midiMap '%s'\n", d.path().filename().string());
		m_mapFiles.push_back(d.path().filename().string());
	}

	u::log::print("[MidiMapper::init] total midimaps found: %d\n", m_mapFiles.size());
}

/* -------------------------------------------------------------------------- */

int MidiMapper::read(const std::string& file)
{
	if (file.empty())
	{
		u::log::print("[MidiMapper::read] midiMap not specified, nothing to do\n");
		return MIDIMAP_NOT_SPECIFIED;
	}

	u::log::print("[MidiMapper::read] reading midiMap file '%s'\n", file);

	std::ifstream ifs(m_mapsPath + file);
	if (!ifs.good())
		return MIDIMAP_UNREADABLE;

	nl::json j = nl::json::parse(ifs);

	m_currentMap.brand  = j[MIDIMAP_KEY_BRAND];
	m_currentMap.device = j[MIDIMAP_KEY_DEVICE];

	if (!readInitCommands(m_currentMap, j))
		return MIDIMAP_UNREADABLE;
	if (readCommand(j, m_currentMap.muteOn, MIDIMAP_KEY_MUTE_ON))
		parse(m_currentMap.muteOn);
	if (readCommand(j, m_currentMap.muteOff, MIDIMAP_KEY_MUTE_OFF))
		parse(m_currentMap.muteOff);
	if (readCommand(j, m_currentMap.soloOn, MIDIMAP_KEY_SOLO_ON))
		parse(m_currentMap.soloOn);
	if (readCommand(j, m_currentMap.soloOff, MIDIMAP_KEY_SOLO_OFF))
		parse(m_currentMap.soloOff);
	if (readCommand(j, m_currentMap.waiting, MIDIMAP_KEY_WAITING))
		parse(m_currentMap.waiting);
	if (readCommand(j, m_currentMap.playing, MIDIMAP_KEY_PLAYING))
		parse(m_currentMap.playing);
	if (readCommand(j, m_currentMap.stopping, MIDIMAP_KEY_STOPPING))
		parse(m_currentMap.stopping);
	if (readCommand(j, m_currentMap.stopped, MIDIMAP_KEY_STOPPED))
		parse(m_currentMap.stopped);
	if (readCommand(j, m_currentMap.playingInaudible, MIDIMAP_KEY_PLAYING_INAUDIBLE))
		parse(m_currentMap.playingInaudible);

	return MIDIMAP_READ_OK;
}

/* -------------------------------------------------------------------------- */

bool MidiMapper::isMessageDefined(const MidiMap::Message& m) const
{
	return m.offset != -1;
}

/* -------------------------------------------------------------------------- */

void MidiMapper::sendInitMessages(const MidiMap& midiMap)
{
	if (!midiMap.isValid())
		return;

	for (const MidiMap::Message& m : midiMap.initCommands)
	{
		if (m.value == 0x0 || m.channel == -1)
			continue;
		MidiEvent e(m.value);
		e.setChannel(m.channel);
		m_kernelMidi.send(e.getRaw());
	}
}

/* -------------------------------------------------------------------------- */

void MidiMapper::sendMidiLightning(uint32_t learnt, const MidiMap::Message& m)
{
	// Skip lightning message if not defined in midi map

	if (!isMessageDefined(m))
	{
		u::log::print("[MidiMapper::sendMidiLightning] message skipped (not defined in midiMap)");
		return;
	}

	u::log::print("[MidiMapper::sendMidiLightning] learnt=0x%X, chan=%d, msg=0x%X, offset=%d\n",
	    learnt, m.channel, m.value, m.offset);

	/* Isolate 'channel' from learnt message and offset it as requested by 'nn' in 
	the midiMap configuration file. */

	uint32_t out = ((learnt & 0x00FF0000) >> 16) << m.offset;

	/* Merge the previously prepared channel into final message, and finally send 
	it. */

	out |= m.value | (m.channel << 24);
	m_kernelMidi.send(out);
}

/* -------------------------------------------------------------------------- */

void MidiMapper::parse(MidiMap::Message& message) const
{
	/* Remove '0x' part from the original string. */

	std::string input = message.valueStr;

	std::size_t f = input.find("0x"); // check if "0x" is there
	if (f != std::string::npos)
		input = message.valueStr.replace(f, 2, "");

	/* Then transform string value into the actual uint32_t value, by parsing
	each char (i.e. nibble) in the original string. Substitute 'n' with
	zeros. */

	std::string output;
	for (unsigned i = 0, p = 24; i < input.length(); i++, p -= 4)
	{
		if (input[i] == 'n')
		{
			output += '0';
			if (message.offset == -1) // do it once
				message.offset = p;
		}
		else
			output += input[i];
	}

	/* From string to uint32_t */

	message.value = strtoul(output.c_str(), nullptr, 16);

	u::log::print("[MidiMapper::parse] parsed chan=%d valueStr=%s value=%#x, offset=%d\n",
	    message.channel, message.valueStr, message.value, message.offset);
}

/* -------------------------------------------------------------------------- */

bool MidiMapper::readCommand(const nl::json& j, MidiMap::Message& m, const std::string& key) const
{
	if (j.find(key) == j.end())
		return false;

	const nl::json& jc = j[key];

	m.channel  = jc[MIDIMAP_KEY_CHANNEL];
	m.valueStr = jc[MIDIMAP_KEY_MESSAGE];

	return true;
}
/* -------------------------------------------------------------------------- */

bool MidiMapper::readInitCommands(MidiMap& midiMap, const nl::json& j)
{
	if (j.find(MIDIMAP_KEY_INIT_COMMANDS) == j.end())
		return false;

	for (const auto& jc : j[MIDIMAP_KEY_INIT_COMMANDS])
	{
		MidiMap::Message m;
		m.channel  = jc[MIDIMAP_KEY_CHANNEL];
		m.valueStr = jc[MIDIMAP_KEY_MESSAGE];
		m.value    = strtoul(m.valueStr.c_str(), nullptr, 16);

		midiMap.initCommands.push_back(m);
	}

	return true;
}
} // namespace giada::m