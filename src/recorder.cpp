/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * recorder
 * Action recorder.
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


#include <math.h>
#include "recorder.h"
#include "const.h"
#include "utils.h"
#include "mixer.h"
#include "mixerHandler.h"
#include "kernelAudio.h"
#include "utils.h"
#include "patch.h"
#include "conf.h"


extern Mixer G_Mixer;
extern Patch f_patch;
extern Conf	 G_Conf;


namespace recorder {

gVector<int> frames;
gVector< gVector<action*> > global;
gVector<action*>  actions;

bool active = false;
bool chanActive[MAX_NUM_CHAN];
bool chanEvents[MAX_NUM_CHAN];
bool sortedActions = false;

composite cmp;


/* ------------------------------------------------------------------ */


void init() {
	sortedActions = false;
	active = false;
	clearAll();
}


/* ------------------------------------------------------------------ */


bool canRec(int ch) {

	/* NO recording if:
	 * recorder is inactive
	 * mixer is not running
	 * mixer is recording a take in this channel ch
	 * channel is empty */

	if (!active || !G_Mixer.running || G_Mixer.chanInput == ch || G_Mixer.channels.at(ch)->wave == NULL)
		return 0;
	return 1;
}


/* ------------------------------------------------------------------ */


void rec(int c, char act, int frame) {

	/* allocating the action */

	action *a = (action*) malloc(sizeof(action));
	a->chan  = c;
	a->type  = act;
	a->frame = frame;

	/* check if the frame exists in the stack. If it exists, we don't extend
	 * the stack, but we add (or push) a new action to it. */

	int frameToExpand = frames.size;
	for (int i=0; i<frameToExpand; i++)
		if (frames.at(i) == frame) {
			frameToExpand = i;
			break;
		}

	/* espansione dello stack frames nel caso l'azione ricada in frame
	 * non precedentemente memorizzati (frameToExpand == frames.size).
	 * Espandere frames è facile, basta aggiungere un frame in coda.
	 * Espandere global è più complesso: bisogna prima allocare una
	 * cella in global (per renderlo parallelo a frames) e poi
	 * inizializzare il suo sub-stack (di action). */

	if (frameToExpand == (int) frames.size) {
		frames.add(frame);
		global.add(actions);							// array of actions added
		global.at(global.size-1).add(a);	// action added
	}
	else {

		/* no duplicates, please */

		for (unsigned t=0; t<global.at(frameToExpand).size; t++)
			if (global.at(frameToExpand).at(t)->chan == c && global.at(frameToExpand).at(t)->type == act)
				return;
		global.at(frameToExpand).add(a);		// expand array
	}

	/* don't activate the channel (chanActive[c] == false), it's up to
	 * the other layers */

	chanEvents[c] = true;
	sortedActions = false;

	printf("[REC] action %d recorded on frame %d, chan %d\n", act, frame, c);
	//print();
}


/* ------------------------------------------------------------------ */


void clearChan(int ch) {

	printf("[REC] clearing chan %d...\n", ch);

	for (unsigned i=0; i<global.size; i++) {	// for each frame i
		unsigned j=0;
		while (true) {
			if (j == global.at(i).size) break; 	  // for each action j of frame i
			if (global.at(i).at(j)->chan == ch)	{
				free(global.at(i).at(j));
				global.at(i).del(j);
			}
			else
				j++;
		}
	}
	chanEvents[ch] = false;
	optimize();
	//print();
}


/* ------------------------------------------------------------------ */


void clearAction(int ch, char act) {
	printf("[REC] clearing action %d from chan %d...\n", act, ch);
	for (unsigned i=0; i<global.size; i++) {						// for each frame i
		unsigned j=0;
		while (true) {                                   // for each action j of frame i
			if (j == global.at(i).size)
				break;
			if (global.at(i).at(j)->chan == ch && (act & global.at(i).at(j)->type) == global.at(i).at(j)->type)	{ // bitmask
				free(global.at(i).at(j));
				global.at(i).del(j);
			}
			else
				j++;
		}
	}
	chanEvents[ch] = false;
	optimize();
	chanHasEvents(ch);
	//print();
}


/* ------------------------------------------------------------------ */


void deleteAction(int chan, int frame, char type) {

	/* find the frame 'frame' */

	bool found = false;
	for (unsigned i=0; i<frames.size && !found; i++) {
		if (frames.at(i) == frame) {

			/* find the action in frame i */

			for (unsigned j=0; j<global.at(i).size; j++) {
				//action *a = global.at(i).at(j);
				//if (a->chan == chan && (type & a->type) == a->type)
				if (global.at(i).at(j)->chan == chan && global.at(i).at(j)->type == type)
				{
					int lockStatus = 0;
					while (lockStatus == 0) {
						lockStatus = pthread_mutex_trylock(&G_Mixer.mutex_recs);
						if (lockStatus == 0) {
							free(global.at(i).at(j));
							global.at(i).del(j);
							pthread_mutex_unlock(&G_Mixer.mutex_recs);
							found = true;
							break;
						}
						else
							puts("[REC] waiting for mutex...");
					}
				}
			}
		}
	}
	if (found) {
		optimize();
		chanHasEvents(chan);
		printf("[REC] deleted action type=%d at frame %d from chan %d\n", type, frame, chan);
	}
}


/* ------------------------------------------------------------------ */


void deleteActions(int chan, int frame_a, int frame_b, char type) {

	sortActions();
	gVector<int> dels;

	for (unsigned i=0; i<frames.size; i++)
		if (frames.at(i) > frame_a && frames.at(i) < frame_b)
			dels.add(frames.at(i));

	for (unsigned i=0; i<dels.size; i++)
		deleteAction(chan, dels.at(i), type);
}


/* ------------------------------------------------------------------ */


void clearAll() {
	while (global.size > 0) {
		for (unsigned i=0; i<global.size; i++) {
			for (unsigned k=0; k<global.at(i).size; k++)
				free(global.at(i).at(k));									// free action
			global.at(i).clear();												// free action container
			global.del(i);
		}
	}

	for (unsigned i=0; i<MAX_NUM_CHAN; i++) {
		chanEvents[i] = false;
		chanActive[i] = false;
	}

	global.clear();
	frames.clear();
}


/* ------------------------------------------------------------------ */


void optimize() {

	/* do something until the i frame is empty. */

	unsigned i = 0;
	while (true) {
		if (i == global.size) return;
		if (global.at(i).size == 0) {
			global.del(i);
			frames.del(i);
		}
		else
			i++;
	}

	sortActions();
}


/* ------------------------------------------------------------------ */


void sortActions() {
	if (sortedActions)
		return;
	for (unsigned i=0; i<frames.size; i++)
		for (unsigned j=0; j<frames.size; j++)
			if (frames.at(j) > frames.at(i)) {
				frames.swap(j, i);
				global.swap(j, i);
			}
	sortedActions = true;
	//print();
}


/* ------------------------------------------------------------------ */


void updateBpm(float oldval, float newval, int oldquanto) {

	for (unsigned i=0; i<frames.size; i++) {

		float frame  = ((float) frames.at(i)/newval) * oldval;
		frames.at(i) = (int) frame;

		/* the division up here cannot be precise. A new frame can be 44099
		 * and the quantizer set to 44100. That would mean two recs completely
		 * useless. So we compute a reject value ('scarto'): if it's lower
		 * than 6 frames the new frame is collapsed with a quantized frame. */
		/** CHECKME - maybe 6 frames are too low */

		if (frames.at(i) != 0) {
			int scarto = oldquanto % frames.at(i);
			if (scarto > 0 && scarto <= 6)
				frames.at(i) = frames.at(i) + scarto;
		}

		/* never ever have odd frames. */

		if (frames.at(i) % 2 != 0)
			frames.at(i)++;
	}
	//print();
}


/* ------------------------------------------------------------------ */


void updateSamplerate(int systemRate, int patchRate) {

	/* diff ratio: systemRate / patchRate
	 * e.g.  44100 / 96000 = 0.4... */

	if (systemRate == patchRate)
		return;

	printf("[REC] systemRate (%d) != patchRate (%d), converting...\n", systemRate, patchRate);

	float ratio = systemRate / (float) patchRate;
	for (unsigned i=0; i<frames.size; i++) {

		printf("[REC]    oldFrame = %d", frames.at(i));

		float newFrame = frames.at(i);
		newFrame = floorf(newFrame * ratio);

		frames.at(i) = (int) newFrame;

		if (frames.at(i) % 2 != 0)
			frames.at(i)++;

		printf(", newFrame = %d\n", frames.at(i));
	}
}


/* ------------------------------------------------------------------ */


void expand(int old_fpb, int new_fpb) {

	/* this algorithm requires multiple passages if we expand from e.g. 2
	 * to 16 beats, precisely 16 / 2 - 1 = 7 times (-1 is the first group,
	 * which exists yet). If we expand by a non-multiple, the result is zero,
	 * due to float->int implicit cast */

	unsigned pass = (int) (new_fpb / old_fpb) - 1;
	if (pass == 0) pass = 1;

	unsigned init_fs = frames.size;

	for (unsigned z=1; z<=pass; z++) {
		for (unsigned i=0; i<init_fs; i++) {
			unsigned newframe = frames.at(i) + (old_fpb*z);
			frames.add(newframe);
			global.add(actions);
			for (unsigned k=0; k<global.at(i).size; k++)
				rec(global.at(i).at(k)->chan, global.at(i).at(k)->type, newframe);
		}
	}
	printf("[REC] expanded recs\n");
	//print();
}


/* ------------------------------------------------------------------ */


void shrink(int new_fpb) {

	/* easier than expand(): here we delete eveything beyond old_framesPerBars. */

	unsigned i=0;
	while (true) {
		if (i == frames.size) break;

		if (frames.at(i) >= new_fpb) {
			for (unsigned k=0; k<global.at(i).size; k++)
				free(global.at(i).at(k));			// free action
			global.at(i).clear();								// free action container
			global.del(i);											// shrink global
			frames.del(i);											// shrink frames
		}
		else
			i++;
	}
	optimize();
	printf("[REC] shrinked recs\n");
	//print();
}


/* ------------------------------------------------------------------ */


void enableRead(int c) {
	chanActive[c] = true;
}


/* ------------------------------------------------------------------ */


void disableRead(int c) {
	chanActive[c] = false;
	if (G_Conf.recsStopOnChanHalt)
		mh_killChan(c);
}


/* ------------------------------------------------------------------ */


void chanHasEvents(int ch) {
	if (global.size == 0) {
		chanEvents[ch] = false;
		return;
	}
	for (unsigned i=0; i<global.size && !chanEvents[ch]; i++) {
		for (unsigned j=0; j<global.at(i).size && !chanEvents[ch]; j++) {
			if (global.at(i).at(j)->chan == ch)
				chanEvents[ch] = true;
		}
	}
}


/* ------------------------------------------------------------------ */


int getStartActionFrame(int chan, char action, int frame) {

	/* since 'frame' holds the position of the B-action (e.g. MUTE_ON(A) +
	 * MUTE_OFF(B)) we look in the stack for the frame lower than 'frame'.
	 * If frame_a == frame_b returns -2.
	 * If frame_a == not found returns -1 (suspected ring loop). */

	for (unsigned i=frames.size; i>0; i--)
		for (unsigned j=0; j<global.at(i-1).size; j++)
			if (global.at(i-1).at(j)->chan == chan &&
					global.at(i-1).at(j)->type == action)
			{
				if (frames.at(i-1) < frame) {
					//printf("[REC] start Action frame found = %d\n", frames.at(i-1));
					return frames.at(i-1);
				}
				else
				if (frames.at(i-1) == frame) {
					//printf("[REC start Action & end Action collision at %d!\n", frame);
					return -2;
				}
			}

	//puts("[REC] start Action frame NOT found, suspected ring loop!");
	return -1;
}


/* ------------------------------------------------------------------ */


int getEndActionFrame(int chan, char type, int frame) {

	for (unsigned i=0; i<frames.size; i++)
		for (unsigned j=0; j<global.at(i).size; j++)
			if (global.at(i).at(j)->chan == chan &&
					global.at(i).at(j)->type == type &&
					frames.at(i) > frame)
			{
				//printf("[REC] end Action frame found | frame=%d, index=%d\n", frames.at(i), i);
				return frames.at(i);
			}

	//puts("[REC] end Action frame NOT found, suspected ring loop!");
	return -1;
}


/* ------------------------------------------------------------------ */



int getNextAction(int chan, char type, int frame, action **out) {

	sortActions();  // mandatory

	unsigned i=0;
	while (i < frames.size && frames.at(i) <= frame)
		i++;

	if (i == frames.size)   // no further actions
		return -1;

	for (unsigned j=0; j<global.at(i).size; j++) {
		action *a = global.at(i).at(j);
		if (a->chan == chan && (type & a->type) == a->type) {
			*out = global.at(i).at(j);
			return 1;
		}
	}

	return -2;   // no 'type' actions found
}


/* ------------------------------------------------------------------ */


void startOverdub(int ch, char actionMask, int frame) {

	/* prepare the composite struct */

	if (actionMask == ACTION_KEYS) {
		cmp.a1.type = ACTION_KEYPRESS;
		cmp.a2.type = ACTION_KEYREL;
	}
	else {
		cmp.a1.type = ACTION_MUTEON;
		cmp.a2.type = ACTION_MUTEOFF;
	}
	cmp.a1.chan  = ch;
	cmp.a2.chan  = ch;
	cmp.a1.frame = frame;
	// cmp.a2.frame doesn't exist yet

	/* avoid underlying action truncation: if action2.type == nextAction:
	 * you are in the middle of a composite action, truncation needed */

	rec(ch, cmp.a1.type, frame);

	action *act = NULL;
	int res = getNextAction(ch, cmp.a1.type | cmp.a2.type, cmp.a1.frame, &act);
	if (res == 1) {
		if (act->type == cmp.a2.type) {
			int truncFrame = cmp.a1.frame-kernelAudio::realBufsize;
			if (truncFrame < 0)
				truncFrame = 0;
			printf("[REC] add truncation at frame %d, type=%d\n", truncFrame, cmp.a2.type);
			rec(ch, cmp.a2.type, truncFrame);
		}
	}

	chanActive[ch] = false; // don't use disableRead()
}


/* ------------------------------------------------------------------ */


void stopOverdub(int frame) {

	cmp.a2.frame  = frame;
	bool ringLoop = false;
	bool nullLoop = false;

	/* ring loop verification, i.e. a composite action with key_press at
	 * frame N and key_release at frame M, with M <= N */

	if (cmp.a2.frame < cmp.a1.frame) {
		ringLoop = true;
		printf("[REC] ring loop! frame1=%d < frame2=%d\n", cmp.a1.frame, cmp.a2.frame);
		rec(cmp.a2.chan, cmp.a2.type, G_Mixer.totalFrames); 	// record at the end of the sequencer
	}
	else
	if (cmp.a2.frame == cmp.a1.frame) {
		nullLoop = true;
		printf("[REC]  null loop! frame1=%d == frame2=%d\n", cmp.a1.frame, cmp.a2.frame);
		deleteAction(cmp.a1.chan, cmp.a1.frame, cmp.a1.type);
	}

	chanActive[cmp.a2.chan] = true; // don't use disableRead()

	/* remove any nested action between keypress----keyrel, then record */

	if (!nullLoop)
		deleteActions(cmp.a2.chan, cmp.a1.frame, cmp.a2.frame, cmp.a1.type);
		deleteActions(cmp.a2.chan, cmp.a1.frame, cmp.a2.frame, cmp.a2.type);

	if (!ringLoop && !nullLoop) {
		rec(cmp.a2.chan, cmp.a2.type, cmp.a2.frame);

		/* avoid underlying action truncation, if keyrel happens inside a
		* composite action */

		action *act = NULL;
		int res = getNextAction(cmp.a2.chan, cmp.a2.type | cmp.a2.type, cmp.a2.frame, &act);
		if (res == 1) {
			if (act->type == cmp.a2.type) {
				printf("[REC] add truncation at frame %d, type=%d\n", act->frame, act->type);
				deleteAction(act->chan, act->frame, act->type);
			}
		}
	}
}


/* ------------------------------------------------------------------ */


void print() {
	printf("[REC] ** print debug **\n");
	for (unsigned i=0; i<global.size; i++) {
		printf("      [f.%d]", frames.at(i));
		for (unsigned j=0; j<global.at(i).size; j++) {
			printf("[a.%d|c.%d]", global.at(i).at(j)->type, global.at(i).at(j)->chan);
		}
		printf("\n");
	}
}

} // namespace
