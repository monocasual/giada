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


#include <stdio.h>
#include "kernelMidi.h"
#include "channel.h"
#include "pluginHost.h"


extern bool G_midiStatus;

#ifdef WITH_VST
extern PluginHost G_PluginHost;
#endif


namespace kernelMidi {


int        api         = 0;      // one api for both in & out
RtMidiOut *midiOut     = NULL;
RtMidiIn  *midiIn      = NULL;
unsigned   numOutPorts = 0;
unsigned   numInPorts  = 0;

std::vector<unsigned char> msg(3, 0x00);


/* ------------------------------------------------------------------ */


int openOutDevice(int _api, int port) {

	api = _api;

	printf("[KM] using system 0x%x\n", api);

	try {
		midiOut = new RtMidiOut((RtMidi::Api) api, "Giada Output Client");
		G_midiStatus = true;
  }
  catch (RtError &error) {
    printf("[KM] MIDI out device error: %s\n", error.getMessage().c_str());
    G_midiStatus = false;
    return 0;
  }

	/* print output ports */

	numOutPorts = midiOut->getPortCount();
  printf("[KM] %d output MIDI ports found\n", numOutPorts);
  for (unsigned i=0; i<numOutPorts; i++)
		printf("  %d) %s\n", i, getOutPortName(i));

	/* try to open a port, if enabled */

	if (port != -1 && numOutPorts > 0) {
		try {
			midiOut->openPort(port, getOutPortName(port));
			printf("[KM] MIDI out port %d open\n", port);
			return 1;
		}
		catch (RtError &error) {
			printf("[KM] unable to open MIDI out port %d: %s\n", port, error.getMessage().c_str());
			G_midiStatus = false;
			return 0;
		}
	}
	else
		return 2;
}


/* ------------------------------------------------------------------ */


bool hasAPI(int API) {
	std::vector<RtMidi::Api> APIs;
	RtMidi::getCompiledApi(APIs);
	for (unsigned i=0; i<APIs.size(); i++)
		if (APIs.at(i) == API)
			return true;
	return false;
}


/* ------------------------------------------------------------------ */


const char *getOutPortName(unsigned p) {
	try { return midiOut->getPortName(p).c_str(); }
	catch (RtError &error) { return NULL; }
}


/* ------------------------------------------------------------------ */


void send(uint32_t data) {
	if (!G_midiStatus)
		return;
	msg[0] = getB1(data);
	msg[1] = getB2(data);
	msg[2] = getB3(data);
	midiOut->sendMessage(&msg);
	printf("[KM] send msg=0x%X\n", data);
}


/* ------------------------------------------------------------------ */


void send(int b1, int b2, int b3) {
	if (!G_midiStatus)
		return;
		msg[0] = b1;
		msg[1] = b2;
		msg[2] = b3;
		midiOut->sendMessage(&msg);
	printf("[KM] send msg=0x%X\n", getIValue(b1, b2, b3));
}


}  // namespace


