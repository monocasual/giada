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


// A MidiMsg filtering class
// Allows to tell whether MidiMsg is of a defined type
//
// TODO: Knob/slider jitter filter


class MidiMsgFilter
{
	public:

	// Creates a MidiMsgFilter that passes only a given message
	MidiMsgFilter(MidiMsg mm, bool alm = 0);

	// Creates the simplest, fully transparent filter
	// Note it has allow_longer_msg set to true
	MidiMsgFilter();

	// Creates a transparent filter of a given length
	MidiMsgFilter(unsigned int fl, bool alm = 0);

	// Creates a filter defined by length l, and mask and tmpl arrays
	// m_allow_longer_msg is optional
	MidiMsgFilter(unsigned int fl, unsigned char* mask, unsigned char* tmpl,
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
	bool			check(MidiMsg& mm);
	
	// Check a message against a given filter
	friend bool		check(MidiMsg& mm, MidiMsgFilter mmf);

	private:

	// m_template vector contains data to which message is compared
	// m_mask indicates which bits are to be included in comparison
	// m_template and m_mask must always be of equal size	
	std::vector<unsigned char>	m_template;
	std::vector<unsigned char>	m_mask;
	bool 				m_allow_longer_msg;

};

}} // giada::m
#endif
