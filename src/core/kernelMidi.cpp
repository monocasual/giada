/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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


#include "const.h"
#ifdef G_OS_MAC
	#include <RtMidi.h>
#else
	#include <rtmidi/RtMidi.h>
#endif
#include "../utils/log.h"
#include "midiDispatcher.h"
#include "midiMapConf.h"
#include "kernelMidi.h"


using std::string;
using std::vector;


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


static void callback_(double t, vector<unsigned char>* msg, void* data)
{
	if (msg->size() < 3) {
		//gu_log("[KM] MIDI received - unknown signal - size=%d, value=0x", (int) msg->size());
		//for (unsigned i=0; i<msg->size(); i++)
		//	gu_log("%X", (int) msg->at(i));
		//gu_log("\n");
		return;
	}
	midiDispatcher::dispatch(msg->at(0), msg->at(1), msg->at(2));
}


/* -------------------------------------------------------------------------- */


void sendMidiLightningInitMsgs_()
{
	for(unsigned i=0; i<midimap::initCommands.size(); i++) {
		midimap::message_t msg = midimap::initCommands.at(i);
		if (msg.value != 0x0 && msg.channel != -1) {
			gu_log("[KM] MIDI send (init) - Channel %x - Event 0x%X\n", msg.channel, msg.value);
			send(msg.value | G_MIDI_CHANS[msg.channel]);
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
	gu_log("[KM] using system 0x%x\n", api_);
}


/* -------------------------------------------------------------------------- */


int openOutDevice(int port)
{
	try {
		midiOut_ = new RtMidiOut((RtMidi::Api) api_, "Giada MIDI Output");
		status_  = true;
	}
	catch (RtMidiError &error) {
		gu_log("[KM] MIDI out device error: %s\n", error.getMessage().c_str());
		status_ = false;
		return 0;
	}

	/* print output ports */

	numOutPorts_ = midiOut_->getPortCount();
	gu_log("[KM] %d output MIDI ports found\n", numOutPorts_);
	for (unsigned i=0; i<numOutPorts_; i++)
		gu_log("  %d) %s\n", i, getOutPortName(i).c_str());

	/* try to open a port, if enabled */

	if (port != -1 && numOutPorts_ > 0) {
		try {
			midiOut_->openPort(port, getOutPortName(port));
			gu_log("[KM] MIDI out port %d open\n", port);

			/* TODO - it shold send midiLightning message only if there is a map loaded
			and available in midimap:: */

			sendMidiLightningInitMsgs_();
			return 1;
		}
		catch (RtMidiError& error) {
			gu_log("[KM] unable to open MIDI out port %d: %s\n", port, error.getMessage().c_str());
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
		gu_log("[KM] MIDI in device error: %s\n", error.getMessage().c_str());
		status_ = false;
		return 0;
	}

	/* print input ports */

	numInPorts_ = midiIn_->getPortCount();
	gu_log("[KM] %d input MIDI ports found\n", numInPorts_);
	for (unsigned i=0; i<numInPorts_; i++)
		gu_log("  %d) %s\n", i, getInPortName(i).c_str());

	/* try to open a port, if enabled */

	if (port != -1 && numInPorts_ > 0) {
		try {
			midiIn_->openPort(port, getInPortName(port));
			midiIn_->ignoreTypes(true, false, true); // ignore all system/time msgs, for now
			gu_log("[KM] MIDI in port %d open\n", port);
			midiIn_->setCallback(&callback_);
			return 1;
		}
		catch (RtMidiError& error) {
			gu_log("[KM] unable to open MIDI in port %d: %s\n", port, error.getMessage().c_str());
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
	vector<RtMidi::Api> APIs;
	RtMidi::getCompiledApi(APIs);
	for (unsigned i=0; i<APIs.size(); i++)
		if (APIs.at(i) == API)
			return true;
	return false;
}


/* -------------------------------------------------------------------------- */


string getOutPortName(unsigned p)
{
	try { return midiOut_->getPortName(p); }
	catch (RtMidiError &error) { return ""; }
}

string getInPortName(unsigned p)
{
	try { return midiIn_->getPortName(p); }
	catch (RtMidiError &error) { return ""; }
}


/* -------------------------------------------------------------------------- */


void send(uint32_t data)
{
	if (!status_)
		return;

	vector<unsigned char> msg(1, getB1(data));
	msg.push_back(getB2(data));
	msg.push_back(getB3(data));

	midiOut_->sendMessage(&msg);
	gu_log("[KM] send msg=0x%X (%X %X %X)\n", data, msg[0], msg[1], msg[2]);
}


/* -------------------------------------------------------------------------- */


void send(int b1, int b2, int b3)
{
	if (!status_)
		return;

	vector<unsigned char> msg(1, b1);

	if (b2 != -1)
		msg.push_back(b2);
	if (b3 != -1)
		msg.push_back(b3);

	midiOut_->sendMessage(&msg);
	//gu_log("[KM] send msg=(%X %X %X)\n", b1, b2, b3);
}


/* -------------------------------------------------------------------------- */


void sendMidiLightning(uint32_t learn, const midimap::message_t& msg)
{
	// Skip lightning message if not defined in midi map

	if (!midimap::isDefined(msg))
	{
		gu_log("[KM] message skipped (not defined in midimap)");
		return;
	}

	gu_log("[KM] learn=%#X, chan=%d, msg=%#X, offset=%d\n", learn, msg.channel, 
		msg.value, msg.offset);

	/* Isolate 'channel' from learnt message and offset it as requested by 'nn' in 
	the midimap configuration file. */
	uint32_t out = ((learn & 0x00FF0000) >> 16) << msg.offset;

	/* Merge the previously prepared channel into final message, and finally send 
	it. */
	out |= msg.value | (msg.channel << 24);
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


/* -------------------------------------------------------------------------- */


uint32_t setChannel(uint32_t iValue, int channel)
{
	uint32_t chanMask = 0xF << 24;
	return (iValue & (~chanMask)) | (channel << 24);
}

}}}; // giada::m::kernelMidi::
