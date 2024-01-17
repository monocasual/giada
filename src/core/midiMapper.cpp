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

#include "core/midiMapper.h"
#include "core/const.h"
#include "core/kernelMidi.h"
#include "core/midiEvent.h"
#include "utils/fs.h"
#include "utils/log.h"
#include "utils/string.h"
#include <cstddef>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <string>
#include <vector>
#ifdef WITH_TESTS
#include "tests/mocks/kernelMidiMock.h"
#endif

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

template <typename KernelMidiI>
MidiMapper<KernelMidiI>::MidiMapper(KernelMidiI& k)
: m_kernelMidi(k)
{
	m_mapsPath = u::fs::getMidiMapsPath();
}

/* -------------------------------------------------------------------------- */

template <typename KernelMidiI>
void MidiMapper<KernelMidiI>::init()
{
	Mapper::init();
	u::log::print("[MidiMapper::init] total midimaps found: {}\n", m_mapFiles.size());
}

/* -------------------------------------------------------------------------- */

template <typename KernelMidiI>
int MidiMapper<KernelMidiI>::read(const std::string& file)
{
	std::optional<nl::json> res = Mapper::read(file);
	if (!res)
	{
		u::log::print("[MidiMapper::read] MIDI map read failed!\n");
		return G_FILE_UNREADABLE;
	}

	nl::json j = res.value();

	currentMap.brand  = j[MIDIMAP_KEY_BRAND];
	currentMap.device = j[MIDIMAP_KEY_DEVICE];

	if (!readInitCommands(currentMap, j))
		return G_FILE_UNREADABLE;
	if (readCommand(j, currentMap.muteOn, MIDIMAP_KEY_MUTE_ON))
		parse(currentMap.muteOn);
	if (readCommand(j, currentMap.muteOff, MIDIMAP_KEY_MUTE_OFF))
		parse(currentMap.muteOff);
	if (readCommand(j, currentMap.soloOn, MIDIMAP_KEY_SOLO_ON))
		parse(currentMap.soloOn);
	if (readCommand(j, currentMap.soloOff, MIDIMAP_KEY_SOLO_OFF))
		parse(currentMap.soloOff);
	if (readCommand(j, currentMap.waiting, MIDIMAP_KEY_WAITING))
		parse(currentMap.waiting);
	if (readCommand(j, currentMap.playing, MIDIMAP_KEY_PLAYING))
		parse(currentMap.playing);
	if (readCommand(j, currentMap.stopping, MIDIMAP_KEY_STOPPING))
		parse(currentMap.stopping);
	if (readCommand(j, currentMap.stopped, MIDIMAP_KEY_STOPPED))
		parse(currentMap.stopped);
	if (readCommand(j, currentMap.playingInaudible, MIDIMAP_KEY_PLAYING_INAUDIBLE))
		parse(currentMap.playingInaudible);

	return G_FILE_OK;
}

/* -------------------------------------------------------------------------- */

template <typename KernelMidiI>
bool MidiMapper<KernelMidiI>::isMessageDefined(const MidiMap::Message& m) const
{
	return m.offset != -1;
}

/* -------------------------------------------------------------------------- */

template <typename KernelMidiI>
void MidiMapper<KernelMidiI>::sendInitMessages() const
{
	if (!currentMap.isValid())
		return;

	for (const MidiMap::Message& m : currentMap.initCommands)
	{
		if (m.value == 0x0 || m.channel == -1)
			continue;
		MidiEvent e = MidiEvent::makeFromRaw(m.value, /*numBytes=*/3);
		e.setChannel(m.channel);
		m_kernelMidi.send(e);
	}
}

/* -------------------------------------------------------------------------- */

template <typename KernelMidiI>
void MidiMapper<KernelMidiI>::sendMidiLightning(uint32_t learnt, const MidiMap::Message& m) const
{
	// Skip lightning message if not defined in midi map

	if (!isMessageDefined(m))
	{
		u::log::print("[MidiMapper::sendMidiLightning] message skipped (not defined in midiMap)\n");
		return;
	}

	u::log::print("[MidiMapper::sendMidiLightning] learnt={:#x}, chan={}, msg={:#x}, offset={}\n",
	    learnt, m.channel, m.value, m.offset);

	/* Isolate 'channel' from learnt message and offset it as requested by 'nn' in 
	the midiMap configuration file. */

	uint32_t out = ((learnt & 0x00FF0000) >> 16) << m.offset;

	/* Merge the previously prepared channel into final message, and finally send 
	it. */

	out |= m.value | (m.channel << 24);

	m_kernelMidi.send(MidiEvent::makeFromRaw(out, /*numBytes=*/3));
}

/* -------------------------------------------------------------------------- */

template <typename KernelMidiI>
void MidiMapper<KernelMidiI>::parse(MidiMap::Message& message) const
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

	u::log::print("[MidiMapper::parse] parsed chan={} valueStr={} value={:#x}, offset={}\n",
	    message.channel, message.valueStr, message.value, message.offset);
}

/* -------------------------------------------------------------------------- */

template <typename KernelMidiI>
bool MidiMapper<KernelMidiI>::readCommand(const nl::json& j, MidiMap::Message& m, const std::string& key) const
{
	if (j.find(key) == j.end())
		return false;

	const nl::json& jc = j[key];

	m.channel  = jc[MIDIMAP_KEY_CHANNEL];
	m.valueStr = jc[MIDIMAP_KEY_MESSAGE];

	return true;
}
/* -------------------------------------------------------------------------- */

template <typename KernelMidiI>
bool MidiMapper<KernelMidiI>::readInitCommands(MidiMap& midiMap, const nl::json& j)
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

template class MidiMapper<KernelMidi>;
#ifdef WITH_TESTS
template class MidiMapper<KernelMidiMock>;
#endif
} // namespace giada::m
