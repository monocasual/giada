/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * mixer
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


#ifndef MIXER_H
#define MIXER_H

#include <stdlib.h>
#include <pthread.h>
#include "const.h"
#include "kernelAudio.h"
#include "utils.h"


class Mixer {
public:

	Mixer();
	~Mixer();
	void init();
	int  close();

	/* loadChannel
	 * add a new channel in channels stack and put a Wave inside of it. Wave
	 * could be NULL == empty new channel */

	struct channel *loadChannel(class Wave *w, char side);
	int      deleteChannel(struct channel *ch);
	void     initChannel(struct channel *ch);
	void     freeChannel(struct channel *ch);

	/* pushChannel
	 * add a new wave to an existing channel. */

	void     pushChannel(class Wave *w, struct channel *ch);

	void chanStop(struct channel *ch);
	void chanReset(struct channel *ch);

	/* fadein
	 * prepare for fade-in process. */

	void fadein(channel *ch, bool internal);

	/* fadeout
	 * do a fadeout and eventually another action when finished. */

	void fadeout(channel *ch, int actionPostFadeout=DO_STOP);

	void xfade(struct channel *ch);

	/* getChanPos
	 * returns the position of an active sample. If EMPTY o MISSING
	 * returns -1. */

	int getChanPos(struct channel *ch);

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

	/* isPlaying
	 * is channel playing? */

	bool isPlaying(struct channel *ch);

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

	void setPitch(struct channel *ch, float val);

	void setChanStart(struct channel *ch, unsigned val);
	void setChanEnd  (struct channel *ch, unsigned val);

	/* mergeVirtualInput
	 * memcpy the virtual channel input in the channel designed for input
	 * recording. Called by mixerHandler on stopInputRec() */

	bool mergeVirtualInput();

	int getChannelIndex(struct channel *ch);

	channel *getChannelByIndex(int i);

	inline channel* getLastChannel() { return channels.at(channels.size-1); }


	/* ---------------------------------------------------------------- */


	enum {    // const - what to do when a fadeout ends
		DO_STOP   = 0x01,
		DO_MUTE   = 0x02,
		DO_MUTE_I = 0x04
	};

	enum {    // const - fade types
		FADEOUT = 0x01,
		XFADE   = 0x02
	};

	gVector<channel*> channels;

	bool   running;
	float *vChanInput;                    // virtual channel for recording
	float *vChanInToOut;                  // virtual channel in->out bridge (hear what you're playin)
	int    frameSize;
	float  outVol;
	float  inVol;
	float  peakOut;
	float  peakIn;
	int    quanto;
	char   quantize;
	bool	 metronome;
	float  bpm;
	int    bars;
	int    beats;
	int    waitRec; // delayComp guard

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
	 * the active channel during a recording. NULL = no channels active */

	channel *chanInput;

	/* inputTracker
	 * position of the sample in the input side (recording) */

	int inputTracker;

	/* inToOut
	 * copy, process and paste the input into the output, in order to
	 * obtain a "hear what you're playing" feature. */

	bool inToOut;

	pthread_mutex_t mutex_recs;
	pthread_mutex_t mutex_plugins;


private:

	/* calcFadeoutStep
	 * allows to do a fadeout even if the sample is almost finished */
	void calcFadeoutStep(struct channel *ch);
};

#endif
