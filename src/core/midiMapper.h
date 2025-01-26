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

#ifndef G_MIDIMAPPER_H
#define G_MIDIMAPPER_H

#include "mapper.h"
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

namespace giada::m
{
class KernelMidi;
#ifdef WITH_TESTS
class KernelMidiMock;
#endif
} // namespace giada::m

namespace giada::m
{
struct MidiMap
{
	struct Message
	{
		int         channel  = 0;
		std::string valueStr = "";
		int         offset   = -1;
		uint32_t    value    = 0;
	};

	/* isValid
	A valid MidiMap must have the brand and the device defined. */

	bool isValid() const;

	std::string          brand;
	std::string          device;
	std::vector<Message> initCommands;
	Message              muteOn;
	Message              muteOff;
	Message              soloOn;
	Message              soloOff;
	Message              waiting;
	Message              playing;
	Message              stopping;
	Message              stopped;
	Message              playingInaudible;
};

/* -------------------------------------------------------------------------- */

template <typename KernelMidiI>
class MidiMapper final : public Mapper
{
public:
	MidiMapper(KernelMidiI&);

	/* init
	Parses the midimap folders and find the available midimaps. */

	void init();

	/* read
	Reads a midimap from file 'file' and sets it as the current one. */

	int read(const std::string& file);

	/* sendInitMessages
	Sends initialization messages from the currently loaded midimap to the
	connected MIDI devices. */

	void sendInitMessages() const;

	/* sendMidiLightning
	Sends a MIDI lightning message defined by 'msg'. */

	void sendMidiLightning(uint32_t learnt, const MidiMap::Message& msg) const;

	/* currentMap
	The current MidiMap selected and loaded. It might be invalid if no midimaps
	have been found. */

	MidiMap currentMap;

private:
	KernelMidiI& m_kernelMidi;

	/* isMessageDefined
	Checks whether a specific message has been defined within a midimap file. */

	bool isMessageDefined(const MidiMap::Message& m) const;

	/* parse
	Takes a string message with 'nn' in it and turns it into a real MIDI value.
	TODO - don't edit message in place! */

	void parse(MidiMap::Message& message) const;

	/* TODO - don't edit midiMap in place! */
	bool readInitCommands(MidiMap& midiMap, const nlohmann::json& j);

	/* TODO - don't edit message in place! */
	bool readCommand(const nlohmann::json& j, MidiMap::Message& m, const std::string& key) const;
};

extern template class MidiMapper<KernelMidi>;
#ifdef WITH_TESTS
extern template class MidiMapper<KernelMidiMock>;
#endif
} // namespace giada::m

#endif
