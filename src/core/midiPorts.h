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


#ifndef G_MIDIPORTS_H
#define G_MIDIPORTS_H

#include <string>
#include "midiMsg.h"

namespace giada {
namespace m {
namespace midiPorts
{

// init
void init();

/* setApi
Sets the Api in use for both in & out messages. */

void setApi(int api);

// getIn/OutDevices
// Lists available devices that can be connected to ports
std::vector<std::string> getOutDevices();
std::vector<std::string> getInDevices();

// getIn/OutDeviceIndex
// Returns a device number on the list returned by RtMidi
int getOutDeviceIndex(std::string port);
int getInDeviceIndex(std::string port);

/* open/close/in/outPort */
// No argument (default) for 'close' methods closes all ports.

int openOutPort(std::string port);
int openInPort(std::string port);
int closeInPort(std::string port = "");
int closeOutPort(std::string port = "");

/* getOutDeviceName
Returns the name of the port with a given index. */
// TODO: To be removed when we quit numbering ports //

std::string getOutDeviceName(int index);
std::string getInDeviceName(int index);

// midiReceive
// Sends a MIDI message 'mm' to a port named 'recipient'.
void midiReceive(midiMsg mm, std::string recipient);


bool hasAPI(int API);

}}}; // giada::m::midiPorts::


#endif
