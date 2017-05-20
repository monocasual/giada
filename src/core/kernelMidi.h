/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#ifndef G_KERNELMIDI_H
#define G_KERNELMIDI_H


#ifdef __APPLE__  // our Clang still doesn't know about cstdint (c++11 stuff)
	#include <stdint.h>
#else
	#include <cstdint>
#endif
#include <string>
#include <vector>


namespace giada {
namespace m {
namespace kernelMidi
{
typedef void (cb_midiLearn) (uint32_t, void *);

void startMidiLearn(cb_midiLearn *cb, void *data);
void stopMidiLearn();

int getB1(uint32_t iValue);
int getB2(uint32_t iValue);
int getB3(uint32_t iValue);

uint32_t getIValue(int b1, int b2, int b3);

/* setChannel
Changes MIDI channel number inside iValue. Returns new message with updated
channel. */

uint32_t setChannel(uint32_t iValue, int channel);

/* send
 * send a MIDI message 's' (uint32_t). */

void send(uint32_t s);

/* send (2)
 * send separate bytes of MIDI message. */

void send(int b1, int b2=-1, int b3=-1);

/* setApi
 * set the Api in use for both in & out messages. */

void setApi(int api);

/* getStatus
Returns current engine status. */

bool getStatus();

/* open/close/in/outDevice */

int openOutDevice(int port);
int openInDevice(int port);
int closeInDevice();
int closeOutDevice();

/* getIn/OutPortName
 * return the name of the port 'p'. */

std::string getInPortName(unsigned p);
std::string getOutPortName(unsigned p);

unsigned countInPorts();
unsigned countOutPorts();

bool hasAPI(int API);

}}}; // giada::m::kernelMidi::


#endif
