/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * KernelMidi
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013 Giovanni A. Zuliani | Monocasual
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
 * ------------------------------------------------------------------ */


#ifndef KERNELMIDI_H
#define KERNELMIDI_H


#include <stdint.h>
#include <RtMidi.h>
#include "channel.h"


namespace kernelMidi {

	inline int getB1(uint32_t iValue) { return (iValue >> 24) & 0xFF; }
	inline int getB2(uint32_t iValue) { return (iValue >> 16) & 0xFF; }
	inline int getB3(uint32_t iValue) { return (iValue >> 8)  & 0xFF; }

	inline uint32_t getIValue (int b1, int b2, int b3) {
		return (b1 << 24) | (b2 << 16) | (b3 << 8) | (0x00);
	}

	/* send
	 * send a MIDI message 's' (uint32_t) to channel ch. */

	void send(uint32_t s, struct channel *ch);

	/* send (2)
	 * send separate bytes of MIDI message to channel ch. */

	void send(int b1, int b2, int b3, channel *ch);

	int openDevice();
	int closeDevice();

	const char *getOutPortName(unsigned p);

	extern RtMidiOut *midiOut;
	extern unsigned   numPorts;
}

#endif
