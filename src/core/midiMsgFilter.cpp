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

MidiMsgFilter::MidiMsgFilter(const MidiMsg& mm, const std::string& sender,
								bool alm) {

	auto l = mm.getMessageLength();

	m_template = *mm.getMessage();
	m_mask.resize(l, 0xFF);
	m_allow_longer_msg = alm;
	m_sender = sender;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

MidiMsgFilter::MidiMsgFilter() {
	m_allow_longer_msg = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

MidiMsgFilter::MidiMsgFilter(const int& fl, bool alm) {
	setFilterLength(fl);
	m_allow_longer_msg = alm;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

MidiMsgFilter::MidiMsgFilter(const std::vector<unsigned char>& mask,
		const std::vector<unsigned char>& tmpl, bool alm) {
	m_template = tmpl;
	m_mask = mask;
	m_allow_longer_msg = alm;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

MidiMsgFilter::MidiMsgFilter(const MidiMsgFilter& mmf) {
	m_template = mmf.m_template;
	m_mask = mmf.m_mask;
	m_allow_longer_msg = mmf.m_allow_longer_msg;
	m_sender = mmf.m_sender;

	for (auto& mbo : mmf.m_bin_ops) {
		m_bin_ops.push_back({mbo.bo, new MidiMsgFilter(*mbo.mmf)});
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

MidiMsgFilter::~MidiMsgFilter() {

	// Delete underlying MidiMsgFilters that were previously created
	for (auto& mbo : m_bin_ops) {
		delete mbo.mmf;
	}
}

//----------------------------  MEMBER FUNCTIONS  ------------------------------

/*
void MidiMsgFilter::setTemplateByte(const int& n, const unsigned char& b) {

	// If filter is shorter than necessary,
	// it is expanded with transparent bytes
	if (n >= m_mask.size()) 
		MidiMsgFilter::setFilterLength(n+1);

	m_template.at(n) = b;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MidiMsgFilter::setMaskByte(const int& n, const unsigned char& b) {

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

void MidiMsgFilter::allowLongerMsg() {
	m_allow_longer_msg = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MidiMsgFilter::disallowLongerMsg() {
	m_allow_longer_msg = false;
}

*/

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MidiMsgFilter::ignoreByte(unsigned int n, unsigned char mask) {
	if (n >= m_mask.size()) return;
	m_mask.at(n) &= mask;

	// Apply the same to all related filters 
	for (auto& mbo : m_bin_ops) {
		mbo.mmf->ignoreByte(n, mask);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MidiMsgFilter::setFilterLength(const int& fl) {
	m_template.resize(fl, 0x00);
	m_mask.resize(fl, 0x00);

	// Apply the same to all related filters 
	for (auto& mbo : m_bin_ops) {
		mbo.mmf->setFilterLength(fl);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MidiMsgFilter::dump(int level) const{

	if (level < 1) {
		level = 1;
		u::log::print("[MMF::dump]\n");
	}
	std::string tabs; 
	for (int i=0; i<level; i++) tabs+="\t";

	unsigned int fl = m_mask.size();

	u::log::print("%sTemplate:", tabs.c_str());

	for (unsigned int i = 0; i < fl; i++) {
		u::log::print("0x%02X ", m_template.at(i));
	}
	u::log::print(";\tMask:");
	for (unsigned int i = 0; i < fl; i++) {
		u::log::print("0x%02X ", m_mask.at(i));
	}
	u::log::print(";\tLonger Messages: %s",
			m_allow_longer_msg ? "Allowed" : "Disallowed");

	u::log::print(";\tFrom sender:: %s\n",
			m_sender.empty() ? "(Any)" : m_sender.c_str());

	int nextLevel;
	for (auto& mbo : m_bin_ops) {
		nextLevel = (mbo.mmf->m_bin_ops.empty() ? level : level + 1);
		switch (mbo.bo) {
			case MMF_AND:
				u::log::print("%sAND\n", tabs.c_str());
				mbo.mmf->dump(nextLevel);
				break;
			case MMF_OR:
				u::log::print("%sOR\n", tabs.c_str());
				mbo.mmf->dump(nextLevel);
				break;
			case MMF_NOT:
				u::log::print("%sNOT\n", tabs.c_str());
		}
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

bool MidiMsgFilter::check(const MidiMsg& mm) const {

	unsigned int l  = mm.getMessageLength();
	unsigned int fl = m_mask.size();
	unsigned char b;

	bool output = true;

	// First of all, check if sender matches
	if (!m_sender.empty() && (mm.getMessageSender() == m_sender))
		output = false;

	// If message is shorter than a filter, it's clearly not a match
	else if (l < fl) 
		output = false;

	// Message cannot be longer than a filter if allow_longer_msg isn't set
	else if (!m_allow_longer_msg && (l > fl)) 
		output = false;

	else {
		for (unsigned int i = 0; i < fl; i++) {

			b  = mm.getByte(i);
			b ^= m_template.at(i);
			b &= m_mask.at(i);

			// If any meaningful difference is found, return false
			if (b != 0) {
				output = false;
				break;
			}
		}
	}

	// Process underlying filters that were joined in
	// through custom logic operators

	for (auto& mbo : m_bin_ops) {
		switch (mbo.bo) {
			case MMF_AND:
				output = output && (*mbo.mmf << mm);
				break;
			case MMF_OR:
				output = output || (*mbo.mmf << mm);
				break;
			case MMF_NOT:
				output = !output;
		}
	}

	return output;
}

//----------------------------  FRIEND FUNCTIONS  ------------------------------


bool check(const MidiMsg& mm, const MidiMsgFilter& mmf) {
	return mmf.check(mm);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

MidiMsgFilter MMF_Channel(const int& ch) {

	if ((ch > 16) || (ch < 1))
		return MidiMsgFilter();

	unsigned char och = ch - 1;
	return MidiMsgFilter({0x0F}, {och}, true);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

MidiMsgFilter MMF_Note(const int& n) {

	if ((n > 127) || (n < 0))
		return MidiMsgFilter();

	return MidiMsgFilter({0x00, 0x7F}, {0x00, (unsigned char)n}, true);
}


inline MidiMsgFilter MMF_Param(const int& p) {
	return MMF_Note(p);
}
//--------------------- JSON SERIALIZERS AND DESERIALIZERS ---------------------

void to_json(nl::json& j, const MidiMsgFilter& mmf){
	j = nl::json{{"template", mmf.m_template},
			{"mask", mmf.m_mask},
			{"alm", mmf.m_allow_longer_msg},
			{"sender", mmf.m_sender},
			{"binops", mmf.m_bin_ops}};
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void from_json(nl::json& j, MidiMsgFilter& mmf) {
	j.at("template").get_to(mmf.m_template);
	j.at("mask").get_to(mmf.m_mask);
	j.at("alm").get_to(mmf.m_allow_longer_msg);
	j.at("sender").get_to(mmf.m_sender);
	j.at("binops").get_to(mmf.m_bin_ops);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void mbo_to_json(nl::json& j, const MidiMsgFilter::mmfBinOps& mbo) {
	j["bo"] = mbo.bo;
	j["mmf"] = *mbo.mmf;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void mbo_from_json(const nl::json& j, MidiMsgFilter::mmfBinOps& mbo) {
	j.at("bo").get_to(mbo.bo);
	mbo.mmf = new MidiMsgFilter(j["mmf"]);
}

//--------------------------------- OPERATORS ----------------------------------

MidiMsgFilter MidiMsgFilter::operator&(const MidiMsgFilter& mmf) const {
	MidiMsgFilter output = *this;
	if (output._tryMerge(mmf))
		return output;
	else {
		output.m_bin_ops.push_back({MMF_AND, new MidiMsgFilter(mmf)});
		return output;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

MidiMsgFilter MidiMsgFilter::operator|(const MidiMsgFilter& mmf) const {
	MidiMsgFilter output = *this;
	output.m_bin_ops.push_back({MMF_OR, new MidiMsgFilter(mmf)});
	return output;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

MidiMsgFilter MidiMsgFilter::operator!() const {
	MidiMsgFilter output = *this;
	output.m_bin_ops.push_back({MMF_NOT, nullptr});
	return output;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

bool MidiMsgFilter::operator<<(const MidiMsg& mm) const {
	return check(mm);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

bool MidiMsgFilter::operator<(const MidiMsgFilter& mmf) const {
	if (m_template < mmf.m_template)
		return true;
	if (m_mask < mmf.m_mask)
		return true;
	if (!m_allow_longer_msg && mmf.m_allow_longer_msg)
		return true;
	if (m_sender < mmf.m_sender)
		return true;

	if (m_bin_ops.size() < mmf.m_bin_ops.size())
		return true;
	if (m_bin_ops.size() > mmf.m_bin_ops.size())
		return false;

	for (int i = 0; i < m_bin_ops.size(); i++) {
		if (m_bin_ops[i].bo < mmf.m_bin_ops[i].bo)
			return true;
		if (*m_bin_ops[i].mmf < *mmf.m_bin_ops[i].mmf)
			return true;
	}

	return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

bool MidiMsgFilter::operator==(const MidiMsgFilter& mmf) const {
	if (m_template != mmf.m_template)
		return false;
	if (m_mask != mmf.m_mask)
		return false;
	if (m_allow_longer_msg != mmf.m_allow_longer_msg)
		return false;
	if (m_sender != mmf.m_sender)
		return false;

	if (m_bin_ops.size() != mmf.m_bin_ops.size())
		return false;

	for (int i = 0; i < m_bin_ops.size(); i++) {
		if (m_bin_ops[i].bo != mmf.m_bin_ops[i].bo)
			return false;
		if (!(*m_bin_ops[i].mmf == *mmf.m_bin_ops[i].mmf))
			return false;
	}

	return true;
}


//-------------------------- PRIVATE MEMBER FUNCTIONS --------------------------

bool MidiMsgFilter::_tryMerge(const MidiMsgFilter& mmf) {
	
	// Two filters cannot be merged if
	// either of them has already partnered through mmfBinOps
	if (m_bin_ops.size() || mmf.m_bin_ops.size())
		return false;

	// or have incompatible sizes 
	// and the shorter one doesn't allow for longer messages
	if ((m_mask.size() > mmf.m_mask.size()) && !(mmf.m_allow_longer_msg))
		return false;
	if ((m_mask.size() < mmf.m_mask.size()) && !(m_allow_longer_msg))
		return false;

	// Let's just copy these two and get over with it..
	MidiMsgFilter o = *this;
	MidiMsgFilter i = mmf;

	// Equalize their lengths..
	i.setFilterLength(std::max(i.m_mask.size(), o.m_mask.size()));
	o.setFilterLength(std::max(i.m_mask.size(), o.m_mask.size()));

	// If they have conflicting rules, the result is a
	// filter that passes no messages at all (please don't do that)
	for (auto j = 0; j < i.m_mask.size(); j++) {
		if ((i.m_mask[j] & o.m_mask[j]) &
					(i.m_template[j] ^ o.m_template[j])){
			*this = !MMF_ANY;
			return true;
		}
	}

	// Otherwise, this is actually going to be successful
	for (auto j = 0; j < i.m_mask.size(); j++) {
		o.m_template[j] &= o.m_mask[j];
		o.m_template[j] |= (i.m_template[j] & i.m_mask[j]);
		o.m_mask[j] |= i.m_mask[j];
	}

	*this = o;
	return true;

}

}} // giada::m::
