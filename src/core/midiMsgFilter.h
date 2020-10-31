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


#ifndef G_MIDIMSGFILTER_H
#define G_MIDIMSGFILTER_H

#include <memory>
#include <vector>
#include <functional>
#include "midiMsg.h"

#include "deps/json/single_include/nlohmann/json.hpp"
namespace nl = nlohmann;

namespace giada {
namespace m {


// A MidiMsg filtering class
// Allows to tell whether MidiMsg is of a defined type
//
// TODO: Knob/slider jitter filter

// Binary operators enum
enum mmfBinOper {MMF_NOT,
		MMF_AND,
		MMF_OR};

class MidiMsgFilter
{
	public:

	// Creates a MidiMsgFilter that passes only a given message
	MidiMsgFilter(const MidiMsg& mm, bool alm = 0);

	// Creates the simplest, fully transparent filter
	// Note it has allow_longer_msg set to true
	MidiMsgFilter();

	// Creates a transparent filter of a given length
	MidiMsgFilter(const int& fl, bool alm = 0);

	// Creates a filter defined by length l, and mask and tmpl strings 
	// m_allow_longer_msg is optional
	MidiMsgFilter(const std::vector<unsigned char>& mask, 
			const std::vector<unsigned char>& tmpl, bool alm = 0);

	// Copy constructor
	MidiMsgFilter(const MidiMsgFilter& mmf);

	// Destructor
	~MidiMsgFilter();

	// Copy assignment operator (remedy for warnings)
	MidiMsgFilter operator=(MidiMsgFilter mmf) {
		return MidiMsgFilter(mmf);
	}

	// Filter manipulation methods
	// Note the byte indices are zero-based
	// 'n' is byte index
	// 'b' is binary data byte
	// 'shl' is left-shift of input 'b', optional
	// 'fl' is filter length
/*
	void	setTemplateByte(const int& n, const unsigned char& b);
	void	setMaskByte(const int& n, const unsigned char& b);
	void	orTemplateByte(unsigned n, unsigned char b, unsigned shl = 0);
	void	orMaskByte(unsigned n, unsigned char b, unsigned shl = 0);
	void	allowLongerMsg();
	void	disallowLongerMsg();
*/

	// Ignores a given byte, or any subset of bits in it
	// given as a new mask byte (zeros mean "ignore")
	// Note that you cannot use this method to "unignore" any bits.
	void	ignoreByte(unsigned n, unsigned char mask = 0b00000000);
	void	setFilterLength(const int& fl);

	// Dumps filter structure for diagnostic purposes
	// Don't touch that level parameter.
	void		dump(int level = 1) const;

	// Check a message against this filter
	bool		check(const MidiMsg& mm) const;

	// Check a message against a given filter
	friend bool	check(const MidiMsg& mm, const MidiMsgFilter& mmf);

	// Json serialization methods
	friend void		to_json(nl::json& j, const MidiMsgFilter& mmf);
	friend void		from_json(nl::json& j, MidiMsgFilter& mmf);

	// inter-MMF binary operation
	struct mmfBinOps {
		mmfBinOper bo;
		MidiMsgFilter* mmf;
	};

	// Operator overloads
	// & - logical product of filters (and)
	// | - logical sum of filters (or)
	// ! - inverts filter (not)
	// << - checks MidiMsg against this filter :)
	MidiMsgFilter operator&(const MidiMsgFilter& mmf) const {
		MidiMsgFilter output = *this;
		output.m_bin_ops.push_back({MMF_AND, new MidiMsgFilter(mmf)});
		return output;
	}
	MidiMsgFilter operator|(const MidiMsgFilter& mmf) const {
		MidiMsgFilter output = *this;
		output.m_bin_ops.push_back({MMF_OR, new MidiMsgFilter(mmf)});
		return output;
	}
	MidiMsgFilter operator!() const {
		MidiMsgFilter output = *this;
		output.m_bin_ops.push_back({MMF_NOT, nullptr});
		return output;
	}
	bool operator<<(const MidiMsg& mm) const {
		return check(mm);
	}

	private:

	// m_template vector contains data to which message is compared
	// m_mask indicates which bits are to be included in comparison
	// m_template and m_mask must always be of equal size	
	// m_bin_ops contains binary relations and copies of other MMFs
	// as constructed using binary operators by a user.
	std::vector<unsigned char>	m_template;
	std::vector<unsigned char>	m_mask;
	bool 				m_allow_longer_msg;

	std::vector<mmfBinOps>		m_bin_ops;
	
};

void to_json(nl::json& j, const MidiMsgFilter& mmf);
void from_json(nl::json& j, MidiMsgFilter& mmf);
void mbo_to_json(nl::json& j, const MidiMsgFilter::mmfBinOps& mbo);
void mbo_from_json(const nl::json& j, MidiMsgFilter::mmfBinOps& mbo);

//------------------- const MidiMsgFilters for typical uses -------------------	

const MidiMsgFilter MMF_ANY		= MidiMsgFilter();

const MidiMsgFilter MMF_NOTEONOFF	= MidiMsgFilter({0xE0,0,0}, {0x80,0,0});
const MidiMsgFilter MMF_NOTEON		= MidiMsgFilter({0xF0,0,0}, {0x90,0,0});
const MidiMsgFilter MMF_NOTEOFF		= MidiMsgFilter({0xF0,0,0}, {0x80,0,0});
const MidiMsgFilter MMF_CC		= MidiMsgFilter({0xF0,0,0}, {0xB0,0,0});
const MidiMsgFilter MMF_NOTEONOFFCC	= MMF_NOTEONOFF | MMF_CC;

}} //------------------------------- giada::m:: -------------------------------


// The following templates are necessary
// so DispatchTableItem (that includes mmf) could be serialized as well.
namespace nlohmann {
template <>
struct adl_serializer<giada::m::MidiMsgFilter> {

	static void to_json(json& j, const giada::m::MidiMsgFilter& mmf) {
		giada::m::to_json(j, mmf);
	}

	static void from_json(const json& j, giada::m::MidiMsgFilter& mmf) {
		mmf = j.get<giada::m::MidiMsgFilter>();
	}

};

template <>
struct adl_serializer<giada::m::MidiMsgFilter::mmfBinOps> {

	static void to_json(json& j,
			const giada::m::MidiMsgFilter::mmfBinOps& mbo) {
		
		mbo_to_json(j, mbo);
	}

	static void from_json(const json& j,
				giada::m::MidiMsgFilter::mmfBinOps& mbo) {
		mbo_from_json(j, mbo);
	}

};
} // nl

#endif
