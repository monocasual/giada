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

#include "core/midiMsg.h"
#include "core/midiMsgFilter.h"

#include <functional>
#include <cstdint>
#include "core/model/model.h"
#include "core/midiEvent.h"
#include "core/types.h"
// TODO: Clean includes at the end

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
	bool		isSender(std::string s);

	//  -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -   -

	private:

	std::vector<std::string>  m_senders;
	bool			  m_whitelist; // m_senders list is wl/!bl
	std::string		  m_receiver;

};

// reg function for registering receivers of certain messages
void reg(const std::string& s, const MidiMsgFilter& mmf,
					const std::string &r, bool wl = 1);
void reg(const std::vector<std::string>& s, const MidiMsgFilter& mmf,
					const std::string &r, bool wl = 1);
void regEx(const std::string& s, const MidiMsgFilter& mmf,
					const std::string &r, bool wl = 1);
void regEx(const std::vector<std::string>& s, const MidiMsgFilter& mmf,
					const std::string &r, bool wl = 1);
void unreg(const std::string& r);
void unregEx(const std::string& r);

// The ultimate MidiMsg dispatching method
void dispatch(const MidiMsg& mm);

}}} // giada::m::midiDispatcher::


#endif
