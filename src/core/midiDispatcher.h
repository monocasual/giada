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


#ifndef G_MIDI_DISPATCHER_H
#define G_MIDI_DISPATCHER_H

#include <string>
#include <vector>
#include <functional>

#include "core/midiMsg.h"
#include "core/midiMsgFilter.h"

#include "utils/vector.h"

#include "deps/json/single_include/nlohmann/json.hpp"
namespace nl = nlohmann;

namespace giada {
namespace m {
namespace midiDispatcher
{
class DispatchTableItem{

	public:

	// mmf is public to let us manipulate filter easily
	// using its member functions
	MidiMsgFilter			mmf;

	// Constructors, pretty much straightforward
	// They copy MidiMsgFilter so these don't need to be persistent
	DispatchTableItem(const std::vector<std::string>& s,
		const MidiMsgFilter& mmf, const std::string& r, bool wl = 1);
	DispatchTableItem(const std::string& s,
		const MidiMsgFilter& mmf, const std::string& r, bool wl = 1);

	// These constructors create empty, transparent filters
	// rather than copying an existing filter.
	DispatchTableItem(const std::vector<std::string>& s,
					const std::string& r, bool wl = 1);
	DispatchTableItem(const std::string& s, 
					const std::string& r, bool wl = 1);

	// Table item manipulation methods
	void	addSender(const std::string& s);
	bool	removeSender(const std::string& s); // ret. m_senders.empty()
	void	setReceiver(const std::string& r);
	void	setBlacklist();
	void	setWhitelist();

	// Checks if a message fits to senders, whitelist and filter 
	bool		check(const MidiMsg& mm);

	// Returns receiver's address
	std::string	getReceiver();

	// Checks if a given address is receiver's address
	bool		isReceiver(const std::string& r);

	// Checks if a given address is in a sender's addresses vector
	// NOTE it ignores 'whitelist' flag
	bool		isSender(const std::string& s);

	// json parser serializers
	friend void	to_json(nl::json& j, const DispatchTableItem& dti);
	friend void	from_json(nl::json& j, DispatchTableItem& dti);

	//  -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

	private:

	std::vector<std::string>  m_senders;
	bool			  m_whitelist; // m_senders list is wl/!bl
	std::string		  m_receiver;

};

// reg function for registering receivers of certain messages
void registerRule(const std::string& s, const MidiMsgFilter& mmf,
					const std::string &r, bool wl = 1);
void registerRule(const std::vector<std::string>& s, const MidiMsgFilter& mmf,
					const std::string &r, bool wl = 1);
void registerExRule(const std::string& s, const MidiMsgFilter& mmf,
					const std::string &r, bool wl = 1);
void registerExRule(const std::vector<std::string>& s, const MidiMsgFilter& mmf,
					const std::string &r, bool wl = 1);
void unregisterRule(const std::string& r);
void unregisterExRule(const std::string& r);

// The ultimate MidiMsg dispatching method
void dispatch(const MidiMsg& mm);

// Direct dispatching method
void dispatchTo(const MidiMsg& mm, const std::string& receiver);

//-------------------------------- CONSTRUCTORS --------------------------------
	
inline DispatchTableItem::DispatchTableItem(const std::vector<std::string>& s,
		const MidiMsgFilter& mmf, const std::string& r, bool wl){
	m_senders			= s;
	m_whitelist 			= wl;
	m_receiver 			= r;
	DispatchTableItem::mmf 		= mmf;
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

inline DispatchTableItem::DispatchTableItem(const std::string& s,
		const MidiMsgFilter& mmf, const std::string& r, bool wl){
	m_senders.push_back(s);
	m_whitelist 			= wl;
	m_receiver 			= r;
	DispatchTableItem::mmf 		= mmf;
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

inline DispatchTableItem::DispatchTableItem(const std::vector<std::string>& s,
					const std::string& r, bool wl){
	m_senders			= s;
	m_whitelist 			= wl;
	m_receiver 			= r;
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

inline DispatchTableItem::DispatchTableItem(const std::string& s,
					const std::string& r, bool wl){
	m_senders.push_back(s);
	m_whitelist 			= wl;
	m_receiver 			= r;
}

//------------------------------ MEMBER FUNCTIONS ------------------------------

inline void DispatchTableItem::addSender(const std::string& s){
	m_senders.push_back(s);
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

inline bool DispatchTableItem::removeSender(const std::string& s){
	
	u::vector::remove(m_senders, s);
	return m_senders.empty();
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

inline void DispatchTableItem::setReceiver(const std::string& r){
	m_receiver = r;
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

inline void DispatchTableItem::setBlacklist(){
	m_whitelist = false;
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

inline void DispatchTableItem::setWhitelist(){
	m_whitelist = true;
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

inline bool DispatchTableItem::check(const MidiMsg& mm){

	// Sender cannot be a receiver of its own message
	if (mm.getMessageSender() == m_receiver) return false;

	// Check message sender
	// if found and on a blacklist, or not found and on a whitelist...
	if (isSender(mm.getMessageSender()) != m_whitelist) return false;

	// Check message body against a filter
	return DispatchTableItem::mmf.check(mm);

}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

inline std::string DispatchTableItem::getReceiver(){
	return m_receiver;
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

inline bool DispatchTableItem::isReceiver(const std::string& r){
	return (m_receiver == r);
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

inline bool DispatchTableItem::isSender(const std::string& s){
	return u::vector::has(m_senders, s);
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

inline void to_json(nl::json& j, const DispatchTableItem& dti){
	j = nl::json{{"senders", dti.m_senders},
			{"wl", dti.m_whitelist},
			{"receiver", dti.m_receiver},
			{"mmf", dti.mmf}};
}

//   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

inline void from_json(nl::json& j, DispatchTableItem& dti){
	j.at("senders").get_to(dti.m_senders);
	j.at("wl").get_to(dti.m_whitelist);
	j.at("receiver").get_to(dti.m_receiver);
	j.at("mmf").get_to(dti.mmf);
}

}}} // giada::m::midiDispatcher::


// The following template is necessary
// so the whole DispatchTables could be serialized 
namespace nlohmann {
template <>
struct adl_serializer<giada::m::midiDispatcher::DispatchTableItem> {

	static void to_json(json& j, 
	const giada::m::midiDispatcher::DispatchTableItem& dti) {
		giada::m::midiDispatcher::to_json(j, dti);
	}

	static giada::m::midiDispatcher::DispatchTableItem 
						from_json(const json& j) {
		return j.get<giada::m::midiDispatcher::DispatchTableItem>();
	}

};
} // nl


#endif
