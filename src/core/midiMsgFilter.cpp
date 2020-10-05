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
#include <vector>

namespace giada {
namespace m {

//------------------------------  CONSTRUCTORS  --------------------------------

MidiMsgFilter::MidiMsgFilter(MidiMsg mm, bool alm){ 
	
	unsigned l = mm.getMessageLength();
	
	for (unsigned int i=0; i<l; i++){
		m_template.push_back(mm.getByte(i));
		m_mask.push_back(0xFF);
	}

	m_allow_longer_msg = alm;
	m_extra_filter = ([](MidiMsg){return true;});
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

MidiMsgFilter::MidiMsgFilter(){

	m_allow_longer_msg = true;
	m_extra_filter = ([](MidiMsg){return true;});
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

MidiMsgFilter::MidiMsgFilter(unsigned int fl, bool alm,
					std::function<bool(MidiMsg)> ef){

	for (unsigned int i=0; i<fl; i++){
		m_template.push_back(0);
		m_mask.push_back(0);
	}

	m_allow_longer_msg = alm;
	m_extra_filter = (ef ? ef : ([](MidiMsg){return true;}));
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

MidiMsgFilter::MidiMsgFilter(unsigned int fl, std::string mask, 
		std::string tmpl, bool alm, std::function<bool(MidiMsg)> ef){

	for (unsigned int i=0; i<fl; i++){
		m_template.push_back(tmpl[i]);
		m_mask.push_back(mask[i]);
	}

	m_allow_longer_msg = alm;
	m_extra_filter = (ef ? ef : ([](MidiMsg){return true;}));
}

//----------------------------  MEMBER FUNCTIONS  ------------------------------

void MidiMsgFilter::setTemplateByte(unsigned int n, unsigned char b){
	
	// If filter is shorter than necessary,
	// it is expanded with transparent bytes
	if (n >= m_mask.size()) 
		MidiMsgFilter::setFilterLength(n+1);

	m_template.at(n) = b;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MidiMsgFilter::setMaskByte(unsigned int n, unsigned char b){
	
	// If filter is shorter than necessary,
	// it is expanded with transparent bytes
	if (n >= m_mask.size()) 
		MidiMsgFilter::setFilterLength(n+1);
	
	m_mask.at(n) = b;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MidiMsgFilter::orTemplateByte(unsigned int n, unsigned char b,
							unsigned int shl){

	// If filter is shorter than necessary,
	// it is expanded with transparent bytes
	if (n >= m_mask.size()) 
		MidiMsgFilter::setFilterLength(n-1);

	m_template.at(n) |= (b << shl);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MidiMsgFilter::orMaskByte(unsigned int n, unsigned char b,
							unsigned int shl){
	
	// If filter is shorter than necessary,
	// it is expanded with transparent bytes
	if (n >= m_mask.size()) 
		MidiMsgFilter::setFilterLength(n-1);
	
	m_mask.at(n) |= (b << shl);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MidiMsgFilter::ignoreByte(unsigned int n){
	if (n >= m_mask.size()) return;
	m_mask.at(n) = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MidiMsgFilter::setFilterLength(unsigned int fl){

	unsigned int l = m_mask.size();
	
	// Truncate too long filters
	for (; l > fl; l--){
		m_template.pop_back();
		m_mask.pop_back();
	}

	// Expand too short filters with transparent chunks (mask = 0)
	for (; l < fl; l++){
		m_template.push_back(0);
		m_mask.push_back(0);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MidiMsgFilter::allowLongerMsg(){
	m_allow_longer_msg = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MidiMsgFilter::disallowLongerMsg(){
	m_allow_longer_msg = false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void MidiMsgFilter::setChannel(unsigned ch){

	// If filter is completely empty, expand it to 1 byte
	if (m_mask.size() < 1) setFilterLength(1);

	if ((ch >=1) and (ch <=16)){
		m_template[0] &= 0xF0;
		m_template[0] += ch - 1;
		m_mask[0]     |= 0x0F;
	}
	else {
		m_mask[0]     &= 0xF0;
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

bool MidiMsgFilter::check(const MidiMsg& mm) const{

	unsigned int l  = mm.getMessageLength();
	unsigned int fl = m_mask.size();
	unsigned char b;

	// If message is shorter than a filter, it's clearly not a match
	if (l < fl) return false;

	// Message cannot be longer than a filter if m_allow_longer_msg isn't set
	if ((l > fl) and !(m_allow_longer_msg)) return false;

	for (unsigned int i=0; i<fl; i++){
		
		b  = mm.getByte(i);
		b ^= m_template.at(i);
		b &= m_mask.at(i);

		// If any meaningful difference is found, return false
		if (b != 0) return false;
	}

	// All checks succeeded so far, so extra_filter has a final word
	return m_extra_filter(mm);
}

//----------------------------  FRIEND FUNCTIONS  ------------------------------


bool check(const MidiMsg& mm, const MidiMsgFilter& mmf){

	unsigned int l  = mm.getMessageLength();
	unsigned int fl = mmf.m_mask.size();
	unsigned char b;

	// If message is shorter than a filter, it's clearly not a match
	if (l < fl) return false;

	// Message cannot be longer than a filter if m_allow_longer_msg isn't set
	if ((l > fl) and !(mmf.m_allow_longer_msg)) return false;

	for (unsigned int i=0; i<fl; i++){
		
		b  = mm.getByte(i);
		b ^= mmf.m_template.at(i);
		b &= mmf.m_mask.at(i);

		// MIDI words are 7 bits long, so we ignore MSB
		b &= 0x7F;

		// If any meaningful difference is found, return false
		if (b != 0) return false;
	}

	// All checks succeeded so far, so extra_filter has a final word
	return mmf.m_extra_filter(mm);
}

}} // giada::m::
