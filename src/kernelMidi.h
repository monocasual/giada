/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * KernelMidi
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2014 Giovanni A. Zuliani | Monocasual
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

	extern int      api;      // one api for both in & out
	extern unsigned numOutPorts;
	extern unsigned numInPorts;

	typedef void (cb_midiLearn) (uint32_t, void *);

	/* cb_learn
	 * callback prepared by the gdMidiGrabber window and called by
	 * kernelMidi. It contains things to do once the midi message has been
	 * stored. */

	extern cb_midiLearn *cb_learn;
	extern void         *cb_data;

	void startMidiLearn(cb_midiLearn *cb, void *data);
	void stopMidiLearn();

	inline int getB1(uint32_t iValue) { return (iValue >> 24) & 0xFF; }
	inline int getB2(uint32_t iValue) { return (iValue >> 16) & 0xFF; }
	inline int getB3(uint32_t iValue) { return (iValue >> 8)  & 0xFF; }

	inline uint32_t getIValue(int b1, int b2, int b3) {
		return (b1 << 24) | (b2 << 16) | (b3 << 8) | (0x00);
	}

	/* send
	 * send a MIDI message 's' (uint32_t). */

	void send(uint32_t s);

	/* send (2)
	 * send separate bytes of MIDI message. */

	void send(int b1, int b2=-1, int b3=-1);

	/* setApi
	 * set the Api in use for both in & out messages. */

	void setApi(int api);

	/* open/close/in/outDevice */

	int openOutDevice(int port);
	int openInDevice(int port);
	int closeInDevice();
	int closeOutDevice();

	/* getIn/OutPortName
	 * return the name of the port 'p'. */

	const char *getInPortName(unsigned p);
	const char *getOutPortName(unsigned p);

	bool hasAPI(int API);

	/* callback
	 * master callback for input events. */

	void callback(double t, std::vector<unsigned char> *msg, void *data);
	
	std::string getRtMidiVersion();

	/* sendMidi
	 * send Midi event to the outside world. */

	void init();
	void midi_turnLedOn(uint32_t note, uint32_t color);
	void midi_turnLedOff(uint32_t note);
	void midi_startBlink(uint32_t note, uint32_t color);
	void midi_stopBlink(uint32_t note);
}

#endif
