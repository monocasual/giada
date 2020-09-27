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

#include <vector>
#include "midiMsg.h"

namespace giada {
namespace m {


// A midiMsg filtering class
// Allows to tell whether midiMsg is of a defined type
//
// TODO: Knob/slider jitter filter


class midiMsgFilter
{
	private:

	// template_ vector contains data to which message is compared
	// mask_ indicates which bits are to be included in comparison
	// template_ and mask_ must always be of equal size	
	std::vector<unsigned char>	template_;
	std::vector<unsigned char>	mask_;
	bool 				allow_longer_msg_;

	public:

	// Creates a midiMsgFilter that passes only a given message
	midiMsgFilter(midiMsg mm, bool alm = 0);

	// Creates the simplest, fully transparent filter
	// Note it has allow_longer_msg set to true
	midiMsgFilter();

	// Creates a transparent filter of a given length
	midiMsgFilter(unsigned int fl, bool alm = 0);

	// Creates a filter defined by length l, and mask and tmpl arrays
	// allow_longer_msg_ is optional
	midiMsgFilter(unsigned int fl, unsigned char* mask, unsigned char* tmpl,
							bool alm = 0);

	// Filter manipulation methods
	// Note the byte indices are zero-based
	// 'n' is byte index
	// 'b' is binary data byte
	// 'shl' is left-shift of input 'b', optional
	// 'fl' is filter length
	void	setTemplateByte(unsigned n, unsigned char b);
	void	setMaskByte(unsigned n, unsigned char b);
	void	orTemplateByte(unsigned n, unsigned char b, unsigned shl = 0);
	void	orMaskByte(unsigned n, unsigned char b, unsigned shl = 0);
	void	ignoreByte(unsigned n);
	void	setFilterLength(unsigned fl);
	void	allowLongerMsg();
	void	disallowLongerMsg();

	// Check a message against this filter
	bool			check(midiMsg* mm);
	
	// Check a message against a given filter
	friend bool		check(midiMsg* mm, midiMsgFilter mmf);

};

}}; // giada::m
#endif
