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

#include "midiMsg.h"
#include "midiMsgFilter.h"
#include "utils/log.h"
#include <string>
#include <vector>


namespace giada {
namespace m {

//------------------------------  CONSTRUCTORS  --------------------------------

MidiMsg::MidiMsg(const std::string& sender,
				const std::vector<unsigned char>& message) { 
	m_sender = sender;
	m_message = message;

	_fixVelocityZero();
	_fixNoteOffValue();
}


//----------------------------  MEMBER FUNCTIONS  ------------------------------

unsigned char MidiMsg::getByte(const int& n) const {
	if (n >= m_message.size()) return 0;
	if (n < 0) return 0;
	return m_message[n];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

const std::vector<unsigned char>* MidiMsg::getMessage() const {
	return &m_message;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int MidiMsg::getMessageLength() const {
	return m_message.size();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

std::string MidiMsg::getMessageSender() const {
	return m_sender;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

bool MidiMsg::compare(const MidiMsg& mm,
				std::vector<unsigned char> mask) const {
	
	// Messages must be of equal length
	int tml = this->getMessageLength();
	if (tml != mm.getMessageLength())
		return false;
	
	// mask needs to be adjusted to MM lengths
	mask.resize(tml, 0xFF);

	return MidiMsgFilter(mask, *mm.getMessage()) << *this;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MidiMsg::dump() const {
	
	u::log::print("[MM:dump] Data: ");
	for (auto b : m_message) {
		u::log::print("0x%X ", b);
	}
	u::log::print("; Sent by: %s\n", m_sender.c_str());

}

//----------------------------  FRIEND FUNCTIONS  ------------------------------

void to_json(nl::json& j, const MidiMsg& mm){
	j = nl::json{{"sender", mm.m_sender}, {"msg", mm.m_message}};
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void from_json(nl::json& j, MidiMsg& mm) {
	j.at("sender").get_to(mm.m_sender);
	j.at("msg").get_to(mm.m_message);
}

//-------------------------- PRIVATE MEMBER FUNCTIONS --------------------------

void MidiMsg::_fixVelocityZero() {
	if (MMF_NOTEON.check(*this) && (getByte(2) == 0)) {
		m_message[0] &= 0b11101111;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MidiMsg::_fixNoteOffValue() {
	if (MMF_NOTEOFF.check(*this)) {
		m_message[2] = 0;
	}
}

//--------------------------------- OPERATORS ----------------------------------

bool MidiMsg::operator<(const MidiMsg& mm) const{
	int tml = this->getMessageLength();
	if (tml > mm.getMessageLength())
		return false;

	if (tml < mm.getMessageLength())
		return true;

	for (int i = 0; i < tml; i++) {
		if (this->getByte(i) < mm.getByte(i))
			return true;
	}
	return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

bool MidiMsg::operator<<(const MidiMsg& mm) const{

	// Some special cases
	if ((MMF_NOTEONOFF << *this) && (MMF_NOTEONOFF << mm))
		return compare(mm, {0x0F, 0xFF, 0x00});
	
	if ((MMF_CC << *this) && (MMF_CC << mm))
		return compare(mm, {0xFF, 0xFF, 0x00});

	
	// in all other cases, messages should be completely identical
	return compare(mm, {});

}
}} // giada::m::
