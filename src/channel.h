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
	bool 	 mute_s;     // mute status previous to solo on
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
};


#endif
