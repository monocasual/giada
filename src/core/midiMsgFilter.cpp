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
#include "midiMsgFilter.h"
#include <vector>

namespace giada {
namespace m {

//------------------------------  CONSTRUCTORS  --------------------------------

midiMsgFilter::midiMsgFilter(midiMsg mm, bool alm){ 
	
	unsigned l = mm.getMessageLength();
	
	for (unsigned int i=0; i<l; i++){
		midiMsgFilter::template_.push_back(mm.getByte(i));
		midiMsgFilter::mask_.push_back(0xFF);
	}

	midiMsgFilter::allow_longer_msg_ = alm;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

midiMsgFilter::midiMsgFilter(){

//	midiMsgFilter::template_.push_back(0);
//	midiMsgFilter::mask_.push_back(0);
	midiMsgFilter::allow_longer_msg_ = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

midiMsgFilter::midiMsgFilter(unsigned int fl, bool alm){

	for (unsigned int i=0; i<fl; i++){
		midiMsgFilter::template_.push_back(0);
		midiMsgFilter::mask_.push_back(0);
	}

	midiMsgFilter::allow_longer_msg_ = alm;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

midiMsgFilter::midiMsgFilter(unsigned int fl, unsigned char* mask, 
						unsigned char* tmpl, bool alm){

	for (unsigned int i=0; i<fl; i++){
		midiMsgFilter::template_.push_back(tmpl[i]);
		midiMsgFilter::mask_.push_back(mask[i]);
	}

	midiMsgFilter::allow_longer_msg_ = alm;
}

//----------------------------  MEMBER FUNCTIONS  ------------------------------

void midiMsgFilter::setTemplateByte(unsigned int n, unsigned char b){
	
	// If filter is shorter than necessary,
	// it is expanded with transparent bytes
	if (n >= midiMsgFilter::mask_.size()) 
		midiMsgFilter::setFilterLength(n-1);

	midiMsgFilter::template_.at(n) = b;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void midiMsgFilter::setMaskByte(unsigned int n, unsigned char b){
	
	// If filter is shorter than necessary,
	// it is expanded with transparent bytes
	if (n >= midiMsgFilter::mask_.size()) 
		midiMsgFilter::setFilterLength(n-1);
	
	midiMsgFilter::mask_.at(n) = b;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void midiMsgFilter::orTemplateByte(unsigned int n, unsigned char b,
							unsigned int shl){

	// If filter is shorter than necessary,
	// it is expanded with transparent bytes
	if (n >= midiMsgFilter::mask_.size()) 
		midiMsgFilter::setFilterLength(n-1);

	midiMsgFilter::template_.at(n) |= (b << shl);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void midiMsgFilter::orMaskByte(unsigned int n, unsigned char b,
							unsigned int shl){
	
	// If filter is shorter than necessary,
	// it is expanded with transparent bytes
	if (n >= midiMsgFilter::mask_.size()) 
		midiMsgFilter::setFilterLength(n-1);
	
	midiMsgFilter::mask_.at(n) |= (b << shl);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void midiMsgFilter::ignoreByte(unsigned int n){
	if (n >= midiMsgFilter::mask_.size()) return;
	midiMsgFilter::mask_.at(n) = 0;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void midiMsgFilter::setFilterLength(unsigned int fl){

	unsigned int l = midiMsgFilter::mask_.size();
	
	// Truncate too long filters
	for (; l > fl; l--){
		midiMsgFilter::template_.pop_back();
		midiMsgFilter::mask_.pop_back();
	}

	// Expand too short filters with transparent chunks (mask = 0)
	for (; l < fl; l++){
		midiMsgFilter::template_.push_back(0);
		midiMsgFilter::mask_.push_back(0);
	}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void midiMsgFilter::allowLongerMsg(){
	midiMsgFilter::allow_longer_msg_ = true;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void midiMsgFilter::disallowLongerMsg(){
	midiMsgFilter::allow_longer_msg_ = false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

bool midiMsgFilter::check(midiMsg* mm){

	unsigned int l  = mm->getMessageLength();
	unsigned int fl = midiMsgFilter::mask_.size();
	unsigned char b;

	// If message is shorter than a filter, it's clearly not a match
	if (l < fl) return false;

	// Message cannot be longer than a filter if allow_longer_msg_ isn't set
	if ((l > fl) and !(midiMsgFilter::allow_longer_msg_)) return false;

	for (unsigned int i=0; i<fl; i++){
		
		b  = mm->getByte(i);
		b ^= midiMsgFilter::template_.at(i);
		b &= midiMsgFilter::mask_.at(i);

		// MIDI words are 7 bits long, so we ignore MSB
		b &= 0x7F;

		// If any meaningful difference is found, return false
		if (b != 0) return false;
	}

	// All checks succeeded, so it's a match
	return true;
}

//----------------------------  FRIEND FUNCTIONS  ------------------------------


bool check(midiMsg* mm, midiMsgFilter mmf){

	unsigned int l  = mm->getMessageLength();
	unsigned int fl = mmf.mask_.size();
	unsigned char b;

	// If message is shorter than a filter, it's clearly not a match
	if (l < fl) return false;

	// Message cannot be longer than a filter if allow_longer_msg_ isn't set
	if ((l > fl) and !(mmf.allow_longer_msg_)) return false;

	for (unsigned int i=0; i<fl; i++){
		
		b  = mm->getByte(i);
		b ^= mmf.template_.at(i);
		b &= mmf.mask_.at(i);

		// MIDI words are 7 bits long, so we ignore MSB
		b &= 0x7F;

		// If any meaningful difference is found, return false
		if (b != 0) return false;
	}

	// All checks succeeded, so it's a match
	return true;
}

}} // giada::m::
