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
// The default non-full list omits Giada-created ports.
std::vector<std::string> getOutDevices(bool full = false);
std::vector<std::string> getInDevices(bool full = false);

// getIn/OutDeviceIndex
// Returns a device number on the list returned by RtMidi
// By default returns index on a list without Giada's own ports
int getOutDeviceIndex(const std::string& port, bool full = false);
int getInDeviceIndex(const std::string& port, bool full = false);

// getIn/OutDeviceName
// Returns the name of the port with a given index.
// By default an index on a giada-ports-free list is returned

std::string getOutDeviceName(const int& index, bool full = false);
std::string getInDeviceName(const int& index, bool full = false);

/* open/close/in/outPort */
// No argument (default) for 'close' methods closes all ports.

int openOutPort(const std::string& port);
int openInPort(const std::string& port);
int closeInPort(const std::string& port = "");
int closeOutPort(const std::string& port = "");

// getIn/OuPorts
// Lists open ports in desired direction
// Optionally returns vector of addresses rather than names
std::vector<std::string> getOutPorts(bool addr = false);
std::vector<std::string> getInPorts(bool addr = false);

// midiReceive
// Sends a MIDI message 'mm' to a port named 'recipient'.
void midiReceive(const MidiMsg& mm, const std::string& recipient);


bool hasAPI(int API);

}}} // giada::m::midiPorts::


#endif
