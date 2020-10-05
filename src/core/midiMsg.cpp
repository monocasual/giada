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
#include <string>
#include <vector>

namespace giada {
namespace m {

//------------------------------  CONSTRUCTORS  --------------------------------

MidiMsg::MidiMsg(const std::string& sender,
				const std::vector<unsigned char>& message){ 
	m_sender = sender;
	m_message = message;
	fixVelocityZero();
}


//----------------------------  MEMBER FUNCTIONS  ------------------------------

unsigned char MidiMsg::getByte(int n) const{
	if (n >= m_message.size()) return 0;
	return m_message[n];
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

const std::vector<unsigned char>* MidiMsg::getMessage() const{
	return &m_message;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int MidiMsg::getMessageLength() const{
	return m_message.size();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

std::string MidiMsg::getMessageSender() const{
	return m_sender;
}

//-------------------------- PRIVATE MEMBER FUNCTIONS --------------------------

void MidiMsg::fixVelocityZero(){
	if (MMF_NOTEON.check(*this)){
		if (getByte(2) == 0){
			m_message[0] |= 0b00010000;
		}
	}
}

}} // giada::m::
