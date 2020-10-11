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

	m_template = *mm.getMessage();
	m_mask.resize(l, 0xFF);

	m_allow_longer_msg = alm;
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

	if (level <=1 ) u::log::print("[MMF::dump]\n");
	std::string tabs; 
	for (int i=0; i<level; i++) tabs+="\t";

	unsigned int fl = m_mask.size();

	u::log::print("%sTemplate:", tabs.c_str());

	for (unsigned int i = 0; i < fl; i++) {
		u::log::print("%X ", m_template.at(i));
	}
	u::log::print("\tMask:");
	for (unsigned int i = 0; i < fl; i++) {
		u::log::print("%X ", m_mask.at(i));
	}
	u::log::print("\tLonger Messages: %s\n",
			m_allow_longer_msg ? "Allowed" : "Disallowed");

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

	// If message is shorter than a filter, it's clearly not a match
	if (l < fl) 
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

//--------------------- JSON SERIALIZERS AND DESERIALIZERS ---------------------

void to_json(nl::json& j, const MidiMsgFilter& mmf){
	j = nl::json{{"template", mmf.m_template},
			{"mask", mmf.m_mask},
			{"alm", mmf.m_allow_longer_msg},
			{"binops", mmf.m_bin_ops}};
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void from_json(nl::json& j, MidiMsgFilter& mmf) {
	j.at("template").get_to(mmf.m_template);
	j.at("mask").get_to(mmf.m_mask);
	j.at("alm").get_to(mmf.m_allow_longer_msg);
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

}} // giada::m::
