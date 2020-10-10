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

#include "midiMsgFilter.h"
#include "utils/log.h"
#include <vector>

namespace giada {
namespace m {

//------------------------------  CONSTRUCTORS  --------------------------------

MidiMsgFilter::MidiMsgFilter(const MidiMsg& mm, bool alm) { 
	
	auto l = mm.getMessageLength();
	
	for (auto i = 0; i < l; i++) {
		m_template.push_back(mm.getByte(i));
		m_mask.push_back(0xFF);
	}

	m_allow_longer_msg = alm;
	m_lambda_filter = nullptr;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

MidiMsgFilter::MidiMsgFilter() {

	m_allow_longer_msg = true;
	m_lambda_filter = nullptr;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

MidiMsgFilter::MidiMsgFilter(const int& fl, bool alm,
					std::function<bool(MidiMsg)> ef) {

	for (auto i = 0; i < fl; i++) {
		m_template.push_back(0);
		m_mask.push_back(0);
	}

	m_allow_longer_msg = alm;
	m_lambda_filter = (ef ? ef : nullptr);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

MidiMsgFilter::MidiMsgFilter(const std::vector<unsigned char>& mask, 
		const std::vector<unsigned char>& tmpl,
		bool alm, std::function<bool(MidiMsg)> ef) {

		m_template = tmpl;
		m_mask = mask;

	m_allow_longer_msg = alm;
	m_lambda_filter = (ef ? ef : nullptr);
}

//----------------------------  MEMBER FUNCTIONS  ------------------------------
/*
void MidiMsgFilter::setTemplateByte(unsigned int n, unsigned char b) {
	
	// If filter is shorter than necessary,
	// it is expanded with transparent bytes
	if (n >= m_mask.size()) 
		MidiMsgFilter::setFilterLength(n+1);

	m_template.at(n) = b;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MidiMsgFilter::setMaskByte(unsigned int n, unsigned char b) {
	
	// If filter is shorter than necessary,
	// it is expanded with transparent bytes
	if (n >= m_mask.size()) 
		MidiMsgFilter::setFilterLength(n+1);
	
	m_mask.at(n) = b;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MidiMsgFilter::orTemplateByte(unsigned int n, unsigned char b,
							unsigned int shl) {

	// If filter is shorter than necessary,
	// it is expanded with transparent bytes
	if (n >= m_mask.size()) 
		MidiMsgFilter::setFilterLength(n-1);

	m_template.at(n) |= (b << shl);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MidiMsgFilter::orMaskByte(unsigned int n, unsigned char b,
							unsigned int shl) {
	
	// If filter is shorter than necessary,
	// it is expanded with transparent bytes
	if (n >= m_mask.size()) 
		MidiMsgFilter::setFilterLength(n-1);
	
	m_mask.at(n) |= (b << shl);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MidiMsgFilter::ignoreByte(unsigned int n) {
	if (n >= m_mask.size()) return;
	m_mask.at(n) = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MidiMsgFilter::allowLongerMsg() {
	m_allow_longer_msg = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MidiMsgFilter::disallowLongerMsg() {
	m_allow_longer_msg = false;
}

*/

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MidiMsgFilter::setFilterLength(const int& fl) {
	m_template.resize(fl, 0x00);
	m_mask.resize(fl, 0x00);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MidiMsgFilter::setChannel(const unsigned& ch) {

	// If filter is completely empty, expand it to 1 byte
	if (m_mask.size() < 1) setFilterLength(1);

	if ((ch >=1) and (ch <=16)) {
		m_template[0] &= 0xF0;
		m_template[0] += ch - 1;
		m_mask[0]     |= 0x0F;
	}
	else {
		m_mask[0]     &= 0xF0;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MidiMsgFilter::dump() const{

	unsigned int fl = m_mask.size();

	u::log::print("[MMF::dump]\n");
	u::log::print("\tTemplate:");

	for (unsigned int i = 0; i < fl; i++) {
		u::log::print("%X ", m_template.at(i));
	}
	u::log::print("\n\tMask:");
	for (unsigned int i = 0; i < fl; i++) {
		u::log::print("%X ", m_mask.at(i));
	}
	u::log::print("\n\tLonger Messages: %s",
			m_allow_longer_msg ? "Allowed" : "Disallowed");
	u::log::print("\n\tLambda filter: %s\n",
		(m_lambda_filter != nullptr) ? "Defined" : "Not defined");

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

bool MidiMsgFilter::check(const MidiMsg& mm) const {

	unsigned int l  = mm.getMessageLength();
	unsigned int fl = m_mask.size();
	unsigned char b;

	// If message is shorter than a filter, it's clearly not a match
	if (l < fl) 
		return false;

	// Message cannot be longer than a filter if allow_longer_msg isn't set
	if (!m_allow_longer_msg && (l > fl)) 
		return false;

	for (unsigned int i = 0; i < fl; i++) {
		
		b  = mm.getByte(i);
		b ^= m_template.at(i);
		b &= m_mask.at(i);

		// If any meaningful difference is found, return false
		if (b != 0) 
			return false;
	}

	// All checks succeeded so far, so lambda_filter has a final word
	// (if defined)
	if (m_lambda_filter != nullptr)
		return m_lambda_filter(mm);

	return true;
}

//----------------------------  FRIEND FUNCTIONS  ------------------------------


bool check(const MidiMsg& mm, const MidiMsgFilter& mmf) {
	return mmf.check(mm);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void to_json(nl::json& j, const MidiMsgFilter& mmf){
	j = nl::json{{"template", mmf.m_template},
			{"mask", mmf.m_mask},
			{"alm", mmf.m_allow_longer_msg}};
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void from_json(nl::json& j, MidiMsgFilter& mmf) {
	j.at("template").get_to(mmf.m_template);
	j.at("mask").get_to(mmf.m_mask);
	j.at("alm").get_to(mmf.m_allow_longer_msg);
	mmf.m_lambda_filter = nullptr;
}

}} // giada::m::
