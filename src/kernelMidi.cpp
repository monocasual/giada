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


#ifdef WITH_VST
extern PluginHost G_PluginHost;
#endif


namespace kernelMidi {

RtMidiOut *midiOut  = NULL;
unsigned   numPorts = 0;


/* ------------------------------------------------------------------ */


int openDevice() {

	try {
    midiOut = new RtMidiOut();
  }
  catch (RtError &error) {
    printf("[KM] open device error: %s\n", error.getMessage().c_str());
    return 0;
  }

	numPorts = midiOut->getPortCount();

  printf("[KM] %d output MIDI ports found\n", numPorts);

  for (unsigned i=0; i<numPorts; i++)
		printf("  %d) %s\n", i, getOutPortName(i));

	return 1;
}


/* ------------------------------------------------------------------ */


const char *getOutPortName(unsigned p) {
	try { return midiOut->getPortName(p).c_str(); }
	catch (RtError &error) { return NULL; }
}


/* ------------------------------------------------------------------ */


void send(uint32_t msg, channel *ch) {
	printf("[KM] send msg=%X from channel %d\n", msg, ch->index);
#ifdef WITH_VST
	G_PluginHost.addVstMidiEvent(msg, ch);
#endif
}


/* ------------------------------------------------------------------ */


void send(int b1, int b2, int b3, channel *ch) {
	printf("[KM] send msg=%X from channel %d\n", getIValue(b1, b2, b3), ch->index);
#ifdef WITH_VST
	G_PluginHost.addVstMidiEvent(getIValue(b1, b2, b3), ch);
#endif
}

}


