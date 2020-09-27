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
#include <string>
#include <vector>

namespace giada {
namespace m {

//------------------------------  CONSTRUCTORS  --------------------------------

MidiMsg::MidiMsg(std::string sender, std::vector<unsigned char>* message){ 
	m_sender = sender;
	m_message = message;
}


//----------------------------  MEMBER FUNCTIONS  ------------------------------

unsigned char MidiMsg::getByte(unsigned int n){
	if (n >= m_message->size()) return 0;
	return m_message->at(n);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

std::vector<unsigned char>* MidiMsg::getMessage(){
	return m_message;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

unsigned int MidiMsg::getMessageLength(){
	return m_message->size();
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

std::string MidiMsg::getMessageSender(){
	return m_sender;
}
}} // giada::m::
