/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#include "midiMapConf.h"
#include <cstdint>
#include <string>

namespace giada
{
namespace m
{
namespace kernelMidi
{
int getB1(uint32_t iValue);
int getB2(uint32_t iValue);
int getB3(uint32_t iValue);

uint32_t getIValue(int b1, int b2, int b3);

/* send
Sends a MIDI message 's' as uint32_t or as separate bytes. */

void send(uint32_t s);
void send(int b1, int b2 = -1, int b3 = -1);

/* sendMidiLightning
Sends a MIDI lightning message defined by 'msg'. */

void sendMidiLightning(uint32_t learnt, const midimap::Message& msg);

/* setApi
Sets the Api in use for both in & out messages. */

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
Returns the name of the port 'p'. */

std::string getInPortName(unsigned p);
std::string getOutPortName(unsigned p);

unsigned countInPorts();
unsigned countOutPorts();

bool hasAPI(int API);

} // namespace kernelMidi
} // namespace m
} // namespace giada

#endif
