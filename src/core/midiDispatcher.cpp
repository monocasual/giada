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


#include <vector>
#include <string>
#include <sstream>
#include "utils/vector.h"
//#include "core/midiController.h"
//#include "core/midiDevice.h"
#include "core/midiDispatcher.h"
#include "core/midiPorts.h"
#include "core/types.h"

namespace giada {
namespace m {
namespace midiDispatcher
{
//----------------------------------------------------------------------------//
//-------------------- DISPATCH TABLE ITEM MEMBER FUNCTIONS ------------------//


//-------------------------------- CONSTRUCTORS --------------------------------
	
DispatchTableItem::DispatchTableItem(std::vector<std::string>& s,
				MidiMsgFilter& mmf, std::string& r, bool wl){
	m_senders			= s;
	m_whitelist 			= wl;
	m_receiver 			= r;
	DispatchTableItem::mmf 		= mmf;
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

DispatchTableItem::DispatchTableItem(std::string& s,
				MidiMsgFilter& mmf, std::string& r, bool wl){
	m_senders.push_back(s);
	m_whitelist 			= wl;
	m_receiver 			= r;
	DispatchTableItem::mmf 		= mmf;
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

DispatchTableItem::DispatchTableItem(std::vector<std::string>& s,
				std::string& r, bool wl){
	m_senders			= s;
	m_whitelist 			= wl;
	m_receiver 			= r;
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

DispatchTableItem::DispatchTableItem(std::string& s,
				std::string& r, bool wl){
	m_senders.push_back(s);
	m_whitelist 			= wl;
	m_receiver 			= r;
}

//------------------------------ MEMBER FUNCTIONS ------------------------------

void DispatchTableItem::addSender(std::string& s){
	m_senders.push_back(s);
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

bool DispatchTableItem::removeSender(std::string& s){
	
	u::vector::remove(m_senders, s);
	return m_senders.empty();
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

void DispatchTableItem::setReceiver(std::string& r){
	m_receiver = r;
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

void DispatchTableItem::setBlacklist(){
	m_whitelist = false;
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

void DispatchTableItem::setWhitelist(){
	m_whitelist = true;
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

bool DispatchTableItem::check(MidiMsg& mm){

	// Sender cannot be a receiver of its own message
	if (mm.getMessageSender() == m_receiver) return false;

	// Check message sender
	// if found and on a blacklist, or not found and on a whitelist...
	if (this->isSender(mm.getMessageSender()) != m_whitelist) return false;

	// Check message body against a filter
	return DispatchTableItem::mmf.check(mm);

}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

std::string DispatchTableItem::getReceiver(){
	return m_receiver;
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

bool DispatchTableItem::isReceiver(std::string& r){
	return (m_receiver == r);
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

bool DispatchTableItem::isSender(std::string s){
	return u::vector::has(m_senders, s);
}

//----------------- END OF DISPATCH TABLE ITEM MEMBER FUNCTIONS --------------//
//----------------------------------------------------------------------------//


namespace
{

// Dispatch tables store information about which message should go where.
// dispatchTableEx has higher priority and sends message out on the first
// match, ignoring consecutive possible matches.
// dispatchTable allows for multiple matches and forwards messages to all
// matching receivers.
// Other MIDI-related modules can manipulate these tables using
// reg() and unreg() functions.
std::vector<DispatchTableItem> dispatchTableEx;
std::vector<DispatchTableItem> dispatchTable;

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

void _forward(MidiMsg& mm, std::string r){

	// Addresses are a semicolon delimited strings.
	// First part is either:
	// 'p' for ports,
	// 'c' for channels in Giada, or
	// 'm' for internal modules.
	// All others do nothing, but by convention:
	// 'n' sends messages to null ;)
	// Note that only the first character is read from the first part of 
	// an address, so one might as well put "port" or "channel" in there.
	//
	// The second part is an actual address
	// Should any following parts occur, those are for future use.
	
	// Parse the address into 'parsed_r' vector
	std::vector<std::string> parsed_r;
	std::stringstream ss(r);
	while (ss.good()){
		std::string str;
		getline(ss, str, ';');
		parsed_r.push_back(str);
	}

	// Interpret the address and call appropriate midiReceive function
	switch(parsed_r[0][0]){
		case 'p':

			midiPorts::midiReceive(mm, parsed_r[1]);		
			break;

/*
 * Here messages will be send to channels directly
 * Not implemented at this stage, midiController does that the old way
 
		case 'c':
			ID _id;
			_id = std::stoi(parsed_r[1]);
				//TODO
			break;
*/

		case 'm':

			if (parsed_r[1]=="midiController")
				midiController::midiReceive(mm);
			if (parsed_r[1]=="midiDevice")
				midiDevice::midiReceive(mm);
			
	}


}

} //------------------------------- {anonymous} -------------------------------

void reg(std::string s, MidiMsgFilter& mmf, std::string& r, bool wl){
	dispatchTable.push_back(DispatchTableItem(s, mmf, r, wl));
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

void regEx(std::string s, MidiMsgFilter& mmf, std::string& r, bool wl){
	dispatchTableEx.push_back(DispatchTableItem(s, mmf, r, wl));
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

void dispatch(MidiMsg& mm){

	// Consult dispatchTableEx first
	// If a match is found, send message and ignore all the rest
	for (DispatchTableItem& dti : dispatchTableEx){
		if (dti.check(mm)){
			_forward(mm, dti.getReceiver());
			return;
		}
	}

	// Then consult dispatchTable
	// Store recipients that already got this message,
	// to avoid multiple deliveries
	std::vector<std::string> receivers;

	for (DispatchTableItem& dti : dispatchTable){
		if (u::vector::has(receivers, dti.getReceiver())) continue;
		if (dti.check(mm)){
			_forward(mm, dti.getReceiver());
			receivers.push_back(dti.getReceiver());
		}
	}
}


}}} // giada::m::midiDispatcher::

