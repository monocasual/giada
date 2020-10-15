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


#ifndef G_MIDIMSG_H
#define G_MIDIMSG_H

#include <vector>
#include <string>

#include "deps/json/single_include/nlohmann/json.hpp"
namespace nl = nlohmann;

namespace giada {
namespace m {

class MidiMsg
{
	public:
	MidiMsg(const std::string& sender, 
				const std::vector<unsigned char>& message);

	unsigned char				getByte(const int& n) const;
	const std::vector<unsigned char>*	getMessage() const;
	int					getMessageLength() const;
	std::string				getMessageSender() const;
	
	// NoteOnOff/CC messages
	int					getChannel() const;
	int					getNote() const;
	int					getParam() const;
	int					getVelocity() const;
	int					getValue() const;

	bool compare(const MidiMsg& mm, std::vector<unsigned char> mask) const;

	void					dump() const;

	friend void		to_json(nl::json& j, const MidiMsg& mm);
	friend void		from_json(nl::json& j, MidiMsg& mm);

	// An operator necessary for using MidiMsg as map key
	bool operator<(const MidiMsg& mm) const;

	// Checks if messages are close enough to be considered equal
	// Most notably, in case of Note messages, velocity and On/Offness
	// are not important. 
	// Similarly, CC message value is not compared either.
	bool operator<<(const MidiMsg& mm) const;

	private:
	std::string				m_sender;
	std::vector<unsigned char>		m_message;
	
	// Convert NoteOn with zero velocity into NoteOff
	void 					_fixVelocityZero();

	// Set NoteOff value to zero, due to incompatible handling between 
	// ALSA and JACK (and possibly others).
	void 					_fixNoteOffValue();

};

}} // giada::m
#endif
