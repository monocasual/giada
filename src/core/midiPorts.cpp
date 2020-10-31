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


#include <RtMidi.h>
#include "conf.h"
#include "utils/log.h"
#include "midiDispatcher.h"
#include "midiMapConf.h"
#include "kernelMidi.h"
#include "midiMsg.h"
#include "midiPorts.h"
#include <map>

namespace giada {
namespace m {
namespace midiPorts
{
namespace
{
int api_ = 0;

RtMidiOut* midiOut_ = nullptr;	// These two are for maintenance purposes,
RtMidiIn*  midiIn_  = nullptr;	// should always exist and never open ports. 

// The following maps port names with RtMidi[In/Out] objects that
// support connections to them. There is no way of telling where RtMidi port
// is connected to once connection is estabilished,
// so this information must be stored internally.

std::map<std::string, RtMidiOut*> outPorts_;	// These are meant for actual
std::map<std::string, RtMidiIn*> inPorts_;	// communication purposes


static void callback_(double t, std::vector<unsigned char>* msg, void* data)
{
	// "data" pointer should point at a port name string // 
	std::string port = *static_cast<std::string*>(data);

	// A port name shall be converted into the address string
	port = "p;" + port;

	// Creating a nice MidiMsg instance //
	MidiMsg mm = MidiMsg(port, *msg);

	// Passing it to a new midiDispatcher - TODO //
	// something like:
	midiDispatcher::dispatch(mm);
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

// TODO: Legacy function - to be removed in next stages of overhaul
void sendMidiLightningInitMsgs_()
{
	for (const midimap::Message& m : midimap::midimap.initCommands) {
		if (m.value != 0x0 && m.channel != -1) {
			u::log::print("[KM] MIDI send (init) - Channel %x - Event 0x%X\n", m.channel, m.value);
			MidiEvent e(m.value);
			e.setChannel(m.channel);
			kernelMidi::send(e.getRaw());
		}
	}
}

} // {anonymous}


/* -------------------------------------------------------------------------- */

void init()
{
	setApi(conf::conf.midiSystem);
	midiOut_ = new RtMidiOut((RtMidi::Api) api_, "Giada dummy port");
	midiIn_  = new RtMidiIn((RtMidi::Api) api_, "Giada dummy port");

	// TODO: Open In port and out port for Stage 1
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

bool hasAPI(int API)
{
	std::vector<RtMidi::Api> APIs;
	RtMidi::getCompiledApi(APIs);
	for (unsigned i=0; i<APIs.size(); i++)
		if (APIs.at(i) == API)
			return true;
	return false;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

void setApi(int api)
{
	api_ = api;
	u::log::print("[MP] using system 0x%x\n", api_);
}

/* -------------------------------------------------------------------------- */

std::vector<std::string> getOutDevices(bool full){
	
	std::vector<std::string> output;

	int count;
	try {
		// Get port count first //
		count = midiOut_->getPortCount();
		std::string portname;
	
		// Get names of all available ports //
		for (int i=0; i<count; i++) {
			portname = midiOut_->getPortName(i);
			// If Giada port, don't push back to output
			if (!(full)){
				if (portname.rfind("Giada Output", 0) == 0){
					continue;
				}
			}
			output.push_back(portname);
		}
	}
	catch (RtMidiError &error) {
		u::log::print("[MP::getOutDevices] RtMidiError: %s\n",
						error.getMessage().c_str());
	}

	return output;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

std::vector<std::string> getInDevices(bool full){
	
	std::vector<std::string> output;

	int count;
	try {
		// Get port count first //
		count = midiIn_->getPortCount();
		std::string portname;

		// Get names of all available ports //
		for (int i=0; i<count; i++) {
			portname = midiOut_->getPortName(i);
			// If Giada port, don't push back to output
			if (!(full)){
				if (portname.rfind("Giada Input", 0) == 0){
					continue;
				}
			}
			output.push_back(portname);
		}	
	}
	catch (RtMidiError &error) {
		u::log::print("[MP::getInDevices] RtMidiError: %s\n",
						error.getMessage().c_str());
	}

	return output;
}

/* -------------------------------------------------------------------------- */

int getOutDeviceIndex(std::string port){

	std::vector<std::string> devices = getOutDevices(true);

	auto it = find(devices.begin(), devices.end(), port);
	if (it !=devices.end()) {
		return distance(devices.begin(), it);
	}
	return -1;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int getInDeviceIndex(std::string port){

	std::vector<std::string> devices = getInDevices(true);

	auto it = find(devices.begin(), devices.end(), port);
	if (it !=devices.end()) {
		return distance(devices.begin(), it);
	}
	return -1;
}

/* -------------------------------------------------------------------------- */

int openOutPort(std::string port){

	// First let's see if port name is even valid
	int index = getOutDeviceIndex(port);
	if (index < 0) {
		// No such port is available, exiting
		return -1;
	}

	// Let's check if this port is in the map, possibly already open //
	if (outPorts_.count(port) > 0) {

		// This port already exists in the map
		if (outPorts_[port]->isPortOpen()) {
		
			// And this port is indeed open, nothing to do.
			return 2;
		}
		// However if it is in the map but not open, something is wrong
		// Oh well, we'll se what happens next.
	}

	// No entry in map, so we shall create one
	else {
		try {
			outPorts_[port] = new RtMidiOut((RtMidi::Api) api_,
						"Giada Output to " + port);
		}
		catch (RtMidiError& error) {
			u::log::print("[MP::openOutPort] RtMidiError: %s\n",
						error.getMessage().c_str());
			return -2;
		}
	}
	
	// Let's open this port
	try {
		outPorts_[port]->openPort(index, port);
		u::log::print("[MP::openOutPort] MIDI port \"%s\" open\n",
							port.c_str());
		sendMidiLightningInitMsgs_();
		return 1;
	}
	catch (RtMidiError& error) {
		u::log::print("[MP::openOutPort] unable to open port %s: %s\n",
				port.c_str(), error.getMessage().c_str());
		return -3;
	}

}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int openInPort(std::string port){

	// First let's see if port name is even valid
	int index = getInDeviceIndex(port);
	if (index < 0) {
		// No such port is available, exiting
		return -1;
	}

	// Let's check if this port is in the map, possibly already open //
	if (inPorts_.count(port) > 0) {

		// This port already exists in the map
		if (inPorts_[port]->isPortOpen()) {
		
			// And this port is indeed open, nothing to do.
			return 2;
		}
		// However if it is in the map but not open, something is wrong
		// Oh well, we'll se what happens next.
	}

	// No entry in map, so we shall create one
	else {
		try {
			inPorts_[port] = new RtMidiIn((RtMidi::Api) api_,
						"Giada Input from " + port);
		}
		catch (RtMidiError& error) {
			u::log::print("[MP::openInPort] RtMidiError: %s\n",
						error.getMessage().c_str());
			return -2;
		}
	}
	
	// Let's open this port
	try {
		// Create a persistent string with a port name, so callback
		// function knows which port did the data come from. 
		// TODO: How do we dispose of this thing when the port is closed?
		// A unique_ptr would be nice but I think it's not applicable here.
		std::string* p = new std::string(port);

		inPorts_[port]->setCallback(&callback_, p);
		inPorts_[port]->openPort(index, port);
		inPorts_[port]->ignoreTypes(false, false, false);
		u::log::print("[MP::openInPort] MIDI port \"%s\" open\n",
							port.c_str());
		return 1;
	}
	catch (RtMidiError& error) {
		u::log::print("[MP::openInPort] unable to open port %s: %s\n",
				port.c_str(), error.getMessage().c_str());
		return -3;
	}

}

/* -------------------------------------------------------------------------- */

int closeOutPort(std::string port){

	if (port == ""){

		// Create a list of open ports and destroy them all!
		std::vector<std::string> ports_to_destroy = getOutPorts();

		for (unsigned int i = 0; i < ports_to_destroy.size(); i++){
			closeOutPort(ports_to_destroy[i]);
		}
		return 1;
	}
	// Let's check if this port is in the map //
	if (outPorts_.count(port) > 0) {
		// RtMidi port destructor closes connections on its own
		delete outPorts_[port];
		outPorts_.erase(port);
		u::log::print("[MP::closeOutPort] Port \"%s\" closed.\n",
							port.c_str());
		return 0;
	}
	return 2;
	
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

int closeInPort(std::string port){

	if (port == ""){

		// Create a list of open ports and destroy them all!
		std::vector<std::string> ports_to_destroy = getInPorts();

		for (unsigned int i = 0; i < ports_to_destroy.size(); i++){
			closeInPort(ports_to_destroy[i]);
		}
		return 1;
	}
	// Let's check if this port is in the map //
	if (inPorts_.count(port) > 0) {
		// RtMidi port destructor closes connections on its own
		delete inPorts_[port];
		inPorts_.erase(port);
		u::log::print("[MP::closeInPort] Port \"%s\" closed.\n",
							port.c_str());
		return 0;
	}
	return 2;
	
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

std::vector<std::string> getOutPorts(bool addr){
	
	std::vector<std::string> output;

	for (auto const& [name, rtm] : outPorts_){
		if (addr) output.push_back("p;" + name);
		else output.push_back(name);
	}

	return output;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

std::vector<std::string> getInPorts(bool addr){
	
	std::vector<std::string> output;

	for (auto const& [name, rtm] : inPorts_){
		if (addr) output.push_back("p;" + name);
		else output.push_back(name);
	}

	return output;
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

std::string getOutDeviceName(int index){
	if (index < 0) return "";
	try {return midiOut_->getPortName(index);}
	catch (RtMidiError& error) {return "";}
}

// - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

std::string getInDeviceName(int index){
	if (index < 0) return "";
	try {return midiIn_->getPortName(index);}
	catch (RtMidiError& error) {return "";}
}

/* -------------------------------------------------------------------------- */

// This method sends data from Giada to ports - the name follows the global
// naming convention from midiDispatcher's point of view.

void midiReceive(const MidiMsg& mm, const std::string& recipient)
{
	// Every time a message is sent, a port status is checked first.
	int status = openOutPort(recipient);

	if (status >= 0){
		outPorts_[recipient]->sendMessage(mm.getMessage());

		u::log::print("[MP::midiReceive] Message sent to \"%s\". ",
							recipient.c_str());
		u::log::print("Data: %X",mm.getMessage()->at(0));
		int bytes = mm.getMessage()->size();
		for (int i=1; i< bytes; i++){
			u::log::print(", %X", mm.getMessage()->at(i));
		}
		u::log::print("\n");
	}
	else {
		u::log::print("[MP::midiReceive] Sending message to \"%s\" \
		failed (openOutPort returned %d).", recipient.c_str(),
								status);
	}
}


}}} // giada::m::midiPorts::
