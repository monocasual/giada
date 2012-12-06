/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * mixer
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2012 Giovanni A. Zuliani | Monocasual
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

#ifndef MIXER_H
#define MIXER_H

#include <stdlib.h>
#include <pthread.h>
#include "init.h"
#include "const.h"
#include "kernelAudio.h"
#include "wave.h"
#include "gui_utils.h"
#include "recorder.h"
#include "pluginHost.h"


class Mixer {
public:

	Mixer();
	~Mixer();
	void init();
	int  close();

	void loadWave(class Wave *w, int chan);
	void freeWave(int chan);

	void chanStop(int chan);
	void chanReset(int chan);

	void fadein(int chan);

	/* fadeout
	 * does a fadeout and eventually another action when finished. */

	void fadeout(int chan, int actionPostFadeout=DO_STOP);

	void xfade(int chan);

	/* getChanPos
	 * returns the position of an active sample. If EMPTY o MISSING
	 * returns -1. */

	int getChanPos(int chan);

	/* masterPlay
	 * core method (callback) */

	static int masterPlay(
		void *out_buf, void *in_buf, unsigned n_frames,
		double streamTime, RtAudioStreamStatus status, void *userData
	);
	int __masterPlay(void *out_buf, void *in_buf, unsigned n_frames);

	/* updateFrameBars
	 * updates bpm, frames, beats and so on. */

	void updateFrameBars();

	/* isSilent
	 * is mixer silent? */

	bool isSilent();

	void rewind();

	/* updateQuanto
	 * recomputes the quanto between two quantizations */

	void updateQuanto();

	/* hasLogicalSamples
	 * true if 1 or more samples are logical (memory only, such as takes) */

	bool hasLogicalSamples();

	/* hasEditedSamples
	 * true if 1 or more samples was edited via gEditor */

	bool hasEditedSamples();

	/* updatePitch
	 * updates the pitch value and chanStart+chanEnd accordingly */

	void setPitch(int chan, float val);

	void setChanStart(int chan, unsigned val);
	void setChanEnd  (int chan, unsigned val);


	/* mergeVirtualInput
	 * memcpy the virtual channel input in the channel designed for input
	 * recording. Called by mixerHandler on stopInputRec() */

	bool mergeVirtualInput();


	/* ---------------------------------------------------------------- */


	bool     running;
	float   *vChanInput;                    // virtual channel for recording
	Wave    *chan          [MAX_NUM_CHAN];
	float   *vChan				 [MAX_NUM_CHAN];	// virtual channel
	int      chanStatus    [MAX_NUM_CHAN];	// status: see const.h
	unsigned chanTracker   [MAX_NUM_CHAN];  // chan position
	unsigned chanStart     [MAX_NUM_CHAN];
	unsigned chanEnd       [MAX_NUM_CHAN];
	unsigned chanStartTrue [MAX_NUM_CHAN];	// chanStart NOT pitch affected
	unsigned chanEndTrue   [MAX_NUM_CHAN];	// chanend   NOT pitch affected
	float    chanVolume    [MAX_NUM_CHAN];
  float    chanPitch     [MAX_NUM_CHAN];
	float    chanBoost     [MAX_NUM_CHAN];
	float 	 chanPanLeft   [MAX_NUM_CHAN];
	float 	 chanPanRight  [MAX_NUM_CHAN];
	unsigned chanMode      [MAX_NUM_CHAN];  // mode: see const.h

	/** chanMute is completely useless: use chanMuteVol: it's 0.0 when muted
	 *  otherwise 1.0 */

	bool     chanMute      [MAX_NUM_CHAN];

	float    chanMuteVol   [MAX_NUM_CHAN];
	bool     chanQWait     [MAX_NUM_CHAN];  // quantizer wait
	int 		 chanRecStatus [MAX_NUM_CHAN];  // status of the recordings (treat recs as loops)
	float    chanFadein    [MAX_NUM_CHAN];
	bool     fadeoutOn     [MAX_NUM_CHAN];	//
	float    fadeoutVol    [MAX_NUM_CHAN];  // fadeout volume
	unsigned fadeoutTracker[MAX_NUM_CHAN];  // tracker fadeout, xfade only
	float    fadeoutStep   [MAX_NUM_CHAN];  // fadeout decrease
  int      fadeoutType   [MAX_NUM_CHAN];  // xfade or fadeout
  int			 fadeoutEnd    [MAX_NUM_CHAN];  // what to do when fadeout ends

	unsigned frameSize;
	float    outVol;
	float    inVol;
	float    peakOut;
	float    peakIn;
	unsigned quanto;
	char     quantize;
	bool		 metronome;
	float    bpm;
	int   	 bars;
	int      beats;
	int      waitRec; // delayComp guard

	bool docross;			// crossfade guard
	bool rewindWait;	// rewind guard, if quantized

	int framesPerBar;
	int framesPerBeat;
	int totalFrames;
	int actualFrame;

#define TICKSIZE 38
	static float tock[TICKSIZE];
	static float tick[TICKSIZE];
	int  tickTracker, tockTracker;
	bool tickPlay, tockPlay; // 1 = play, 0 = stop

	/* chanInput
	 * the active channel during a recording. -1 = no channels active */
	int chanInput;

	/* inputTracker
	 * position of the sample in the input side (recording) */
	int inputTracker;

	pthread_mutex_t mutex_recs;
	pthread_mutex_t mutex_plugins;


private:

	/* calcFadeoutStep
	 * allows to do a fadeout even if the sample is almost finished */
	void calcFadeoutStep(int ch);
};

#endif
