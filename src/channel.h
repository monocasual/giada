/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * channel
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


#ifndef CHANNEL_H
#define CHANNEL_H


#include "utils.h"

#if 0
#ifdef WITH_VST

/* before including aeffetx(x).h we must define __cdecl, otherwise VST
 * headers can't be compiled correctly. In windows __cdecl is already
 * defined. */

	#ifdef __GNUC__
		#ifndef _WIN32
			#define __cdecl
		#endif
	#endif
	#include "vst/aeffectx.h"
#endif
#endif


struct channel {
	class Wave *wave;

	int    index;
	float *vChan;	     // virtual channel
	int    status;	   // status: see const.h
	char   side;       // left or right column
	int    tracker;    // chan position
	int    start;
	int    end;
	int    startTrue;	 // chanStart NOT pitch affected
	int    endTrue;	   // chanend   NOT pitch affected
	float  volume;     // global volume
	float  volume_i;   // internal volume
	float  volume_d;   // delta volume (for envelope)
  float  pitch;
	float  boost;
	float  panLeft;
	float  panRight;
	int    mode;       // mode: see const.h
	bool   mute_i;     // internal mute
	bool 	 mute_s;     // previous mute status after being solo'd
	bool   mute;       // global mute
	bool   solo;
	bool   qWait;      // quantizer wait
	float  fadein;
	bool   fadeoutOn;
	float  fadeoutVol;      // fadeout volume
	int    fadeoutTracker;  // tracker fadeout, xfade only
	float  fadeoutStep;     // fadeout decrease
  int    fadeoutType;     // xfade or fadeout
  int		 fadeoutEnd;      // what to do when fadeout ends

	/* recorder:: stuff */

	int 	 recStatus;    // status of recordings (treat recs as loops)
  bool   readActions;  // read actions or not
  bool   hasActions;   // has something recorded

  int    key;

  gVector <class Plugin *> plugins;

  class gChannel *guiChannel;

#if 0
#ifdef WITH_VST

	/* VST struct containing MIDI events. When ready, events are sent to
	 * each plugin in the channel.
	 *
	 * Anatomy of VstEvents
	 * --------------------
	 *
	 * VstInt32  numEvents = number of Events in array
	 * VstIntPtr reserved  = zero (Reserved for future use)
	 * VstEvent *events[2] = event pointer array, variable size
	 *
	 * Note that by default VstEvents only holds three events- if you want
	 * it to hold more, create an equivalent struct with a larger array,
	 * and then cast it to a VstEvents object when you've populated it. */

	struct gVstEvents {
    int       numEvents;
    int       reserved;
    VstEvent *events[256];
	} events;

#endif
#endif

};


#endif
