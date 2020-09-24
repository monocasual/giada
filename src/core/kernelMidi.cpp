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
#include "const.h"
#include "utils/log.h"
#include "midiDispatcher.h"
#include "midiMapConf.h"
#include "kernelMidi.h"


namespace giada {
namespace m {
namespace kernelMidi
{
namespace
{
bool status_ = false;
int api_ = 0;
RtMidiOut* midiOut_ = nullptr;
RtMidiIn*  midiIn_  = nullptr;
unsigned numOutPorts_ = 0;
unsigned numInPorts_  = 0;


static void callback_(double /*t*/, std::vector<unsigned char>* msg, void* /*data*/)
{
	if (msg->size() < 3) {
		//u::log::print("[KM] MIDI received - unknown signal - size=%d, value=0x", (int) msg->size());
		//for (unsigned i=0; i<msg->size(); i++)
		//	u::log::print("%X", (int) msg->at(i));
		//u::log::print("\n");
		return;
	}
	midiDispatcher::dispatch(msg->at(0), msg->at(1), msg->at(2));
}


/* -------------------------------------------------------------------------- */


void sendMidiLightningInitMsgs_()
{
	for (const midimap::Message& m : midimap::midimap.initCommands) {
		if (m.value != 0x0 && m.channel != -1) {
			u::log::print("[KM] MIDI send (init) - Channel %x - Event 0x%X\n", m.channel, m.value);
			MidiEvent e(m.value);
			e.setChannel(m.channel);
			send(e.getRaw());
		}
	}
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void setApi(int api)
{
	api_ = api;
	u::log::print("[KM] using system 0x%x\n", api_);
}


/* -------------------------------------------------------------------------- */


int openOutDevice(int port)
{
	try {
		midiOut_ = new RtMidiOut((RtMidi::Api) api_, "Giada MIDI Output");
		status_  = true;
	}
	catch (RtMidiError &error) {
		u::log::print("[KM] MIDI out device error: %s\n", error.getMessage().c_str());
		status_ = false;
		return 0;
	}

	/* print output ports */

	numOutPorts_ = midiOut_->getPortCount();
	u::log::print("[KM] %d output MIDI ports found\n", numOutPorts_);
	for (unsigned i=0; i<numOutPorts_; i++)
		u::log::print("  %d) %s\n", i, getOutPortName(i).c_str());

	/* try to open a port, if enabled */

	if (port != -1 && numOutPorts_ > 0) {
		try {
			midiOut_->openPort(port, getOutPortName(port));
			u::log::print("[KM] MIDI out port %d open\n", port);

			/* TODO - it shold send midiLightning message only if there is a map loaded
			and available in midimap:: */

			sendMidiLightningInitMsgs_();
			return 1;
		}
		catch (RtMidiError& error) {
			u::log::print("[KM] unable to open MIDI out port %d: %s\n", port, error.getMessage().c_str());
			status_ = false;
			return 0;
		}
	}
	else
		return 2;
}


/* -------------------------------------------------------------------------- */


int openInDevice(int port)
{
	try {
		midiIn_ = new RtMidiIn((RtMidi::Api) api_, "Giada MIDI input");
		status_ = true;
	}
	catch (RtMidiError &error) {
		u::log::print("[KM] MIDI in device error: %s\n", error.getMessage().c_str());
		status_ = false;
		return 0;
	}

	/* print input ports */

	numInPorts_ = midiIn_->getPortCount();
	u::log::print("[KM] %d input MIDI ports found\n", numInPorts_);
	for (unsigned i=0; i<numInPorts_; i++)
		u::log::print("  %d) %s\n", i, getInPortName(i).c_str());

	/* try to open a port, if enabled */

	if (port != -1 && numInPorts_ > 0) {
		try {
			midiIn_->openPort(port, getInPortName(port));
			midiIn_->ignoreTypes(true, false, true); // ignore all system/time msgs, for now
			u::log::print("[KM] MIDI in port %d open\n", port);
			midiIn_->setCallback(&callback_);
			return 1;
		}
		catch (RtMidiError& error) {
			u::log::print("[KM] unable to open MIDI in port %d: %s\n", port, error.getMessage().c_str());
			status_ = false;
			return 0;
		}
	}
	else
		return 2;
}


/* -------------------------------------------------------------------------- */


bool hasAPI(int API)
{
	std::vector<RtMidi::Api> APIs;
	RtMidi::getCompiledApi(APIs);
	for (unsigned i=0; i<APIs.size(); i++)
		if (APIs.at(i) == API)
			return true;
	return false;
}


/* -------------------------------------------------------------------------- */


std::string getOutPortName(unsigned p)
{
	try { return midiOut_->getPortName(p); }
	catch (RtMidiError &error) { return ""; }
}

std::string getInPortName(unsigned p)
{
	try { return midiIn_->getPortName(p); }
	catch (RtMidiError &error) { return ""; }
}


/* -------------------------------------------------------------------------- */


void send(uint32_t data)
{
	if (!status_)
		return;

	std::vector<unsigned char> msg(1, getB1(data));
	msg.push_back(getB2(data));
	msg.push_back(getB3(data));

	midiOut_->sendMessage(&msg);
	u::log::print("[KM::send] send msg=0x%X (%X %X %X)\n", data, msg[0], msg[1], msg[2]);
}


/* -------------------------------------------------------------------------- */


void send(int b1, int b2, int b3)
{
	if (!status_)
		return;

	std::vector<unsigned char> msg(1, b1);

	if (b2 != -1)
		msg.push_back(b2);
	if (b3 != -1)
		msg.push_back(b3);

	midiOut_->sendMessage(&msg);
	u::log::print("[KM::send] send msg=(%X %X %X)\n", b1, b2, b3);
}


/* -------------------------------------------------------------------------- */


void sendMidiLightning(uint32_t learnt, const midimap::Message& m)
{
	// Skip lightning message if not defined in midi map

	if (!midimap::isDefined(m))
	{
		u::log::print("[KM::sendMidiLightning] message skipped (not defined in midimap)");
		return;
	}

	u::log::print("[KM::sendMidiLightning] learnt=0x%X, chan=%d, msg=0x%X, offset=%d\n", 
		learnt, m.channel, m.value, m.offset);

	/* Isolate 'channel' from learnt message and offset it as requested by 'nn' in 
	the midimap configuration file. */

	uint32_t out = ((learnt & 0x00FF0000) >> 16) << m.offset;

	/* Merge the previously prepared channel into final message, and finally send 
	it. */

	out |= m.value | (m.channel << 24);
	send(out);
}


/* -------------------------------------------------------------------------- */


unsigned countInPorts()  { return numInPorts_; }
unsigned countOutPorts() { return numOutPorts_; }
bool getStatus()         { return status_; }


/* -------------------------------------------------------------------------- */


int getB1(uint32_t iValue) { return (iValue >> 24) & 0xFF; }
int getB2(uint32_t iValue) { return (iValue >> 16) & 0xFF; }
int getB3(uint32_t iValue) { return (iValue >> 8)  & 0xFF; }


uint32_t getIValue(int b1, int b2, int b3)
{
	return (b1 << 24) | (b2 << 16) | (b3 << 8) | (0x00);
}
}}} // giada::m::kernelMidi::
