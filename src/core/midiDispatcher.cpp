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


#include <string>
#include <vector>
#include <list>
#include <mutex>
#include <sstream>
#include "utils/vector.h"
#include "utils/log.h"
#include "core/types.h"
#include "core/midiController.h"
#include "core/midiDevice.h"
#include "core/midiLearner.h"
#include "core/midiSignalCb.h"
#include "core/midiDispatcher.h"
#include "core/midiPorts.h"

namespace giada {
namespace m {
namespace midiDispatcher
{

namespace
{

// Dispatch tables store information about which message should go where.
// dispatchTableEx has higher priority and sends message out on the first
// match, ignoring consecutive possible matches.
// dispatchTable allows for multiple matches and forwards messages to all
// matching receivers.
// Other MIDI-related modules can manipulate these tables using
// registerRule() and unregisterRule() functions.
std::list<DispatchTableItem> dispatchTableEx;
std::list<DispatchTableItem> dispatchTable;

// Mutex for each table
std::mutex DTE_mutex;
std::mutex DT_mutex;

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

void forward_(const MidiMsg& mm, const std::string r){

	u::log::print("[MDi::forward_] Forwarding message from %s to: %s...\n",
				mm.getMessageSender().c_str(), r.c_str());

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
		std::getline(ss, str, ';');
		parsed_r.push_back(str);
	}

	// Interpret the address and call appropriate midiReceive function
	switch(parsed_r[0][0]){
		case 'p':
			// Forwarding to port
			midiPorts::midiReceive(mm, parsed_r[1]);		
			return;

/*
 * Here messages will be sent to channels directly
 * Not implemented at this stage, midiController does that the old way
 
		case 'c':
			// Forwarding to channel
			ID _id;
			_id = std::stoi(parsed_r[1]);
				//TODO
			return;
*/

		case 'm':
			// Forwarding to a module
			if (parsed_r[1]=="midiController")
				midiController::midiReceive(mm);
			else if (parsed_r[1]=="midiDevice")
				midiDevice::midiReceive(mm);
			else if (parsed_r[1]=="midiLearner")
				midiLearner::midiReceive(mm);
			else if (parsed_r[1]=="midiSignalCb")
				midiSignalCb::midiReceive(mm);
			return;
	}

	u::log::print("MDi::forward_] Message forwarded to nowhere.\n");

}

} //------------------------------- {anonymous} -------------------------------

void registerRule(const std::string& s, const MidiMsgFilter& mmf,
						const std::string& r, bool wl){
	std::lock_guard lg(DT_mutex);
	dispatchTable.push_back(DispatchTableItem(s, mmf, r, wl));
	u::log::print("[MDi::reg] Registered new DTI (receiver %s).\n", 
								r.c_str());
	mmf.dump();
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

void registerRule(const std::vector<std::string>& s, const MidiMsgFilter& mmf,
						const std::string& r, bool wl){
	std::lock_guard lg(DT_mutex);
	dispatchTable.push_back(DispatchTableItem(s, mmf, r, wl));
	u::log::print("[MDi::reg] Registered new DTI (receiver %s).\n",
								r.c_str());
	mmf.dump();
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

void registerExRule(const std::string& s, const MidiMsgFilter& mmf,
						const std::string& r, bool wl){
	std::lock_guard lg(DTE_mutex);
	dispatchTableEx.push_front(DispatchTableItem(s, mmf, r, wl));
	u::log::print("[MDi::regEx] Registered new DTIEx (receiver %s).\n",
								r.c_str());
	mmf.dump();
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

void registerExRule(const std::vector<std::string>& s,
		const MidiMsgFilter& mmf, const std::string& r, bool wl){
	std::lock_guard lg(DTE_mutex);
	dispatchTableEx.push_front(DispatchTableItem(s, mmf, r, wl));
	u::log::print("[MDi::regEx] Registered new DTIEx (receiver %s).\n",
								r.c_str());
	mmf.dump();
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

void unregisterRule(const std::string& r){
	std::lock_guard lg(DT_mutex);
	dispatchTable.remove_if([&] (DispatchTableItem er)
						{return er.isReceiver(r);});
	u::log::print("[MDi::unreg] Unregistering receiver %s\n", r.c_str());
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

void unregisterExRule(const std::string& r){
	std::lock_guard lg(DTE_mutex);
	dispatchTableEx.remove_if([&] (DispatchTableItem er)
						{return er.isReceiver(r);});
	u::log::print("[MDi::unregEx] Unregistering receiver %s\n",
								r.c_str());
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

void dispatch(const MidiMsg& mm){

	u::log::print("[MDi::dispatch] Dispatching message: ");
	mm.dump();

	// Create a list of receivers to send message to
	// to avoid invoking forward_ with lock guard on.
	std::vector<std::string> receivers;

	// Consult dispatchTableEx first
	// If a match is found, send message and ignore all the rest
	if (true) {		// lock_guard container ;)
		std::lock_guard lg(DTE_mutex);
		for (DispatchTableItem& dti : dispatchTableEx){
			if (dti.check(mm)) {
				u::log::print("[MDi::dispatch] \
						Applied Ex rule.\n");
				receivers.push_back(dti.getReceiver());
				break;
			}
		}
	}
	if (receivers.size() > 0) {
		forward_(mm, receivers[0]);
		return;
	}

	// Then consult dispatchTable
	// Store recipients that already got this message,
	// to avoid multiple deliveries

	if (true) {
		std::lock_guard lg(DT_mutex);
		for (DispatchTableItem& dti : dispatchTable){
			if (u::vector::has(receivers, dti.getReceiver()))
				continue;
			if (dti.check(mm)) {
				receivers.push_back(dti.getReceiver());
			}
		}
	}
	for (std::string& r : receivers) {
		forward_(mm, r);
	}
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

void dispatchTo(const MidiMsg& mm, const std::string& receiver){
	forward_(mm, receiver);
}


}}} // giada::m::midiDispatcher::

