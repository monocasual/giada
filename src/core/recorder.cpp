/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#include <cassert>
#include <cmath>
#include "../utils/log.h"
#include "const.h"
#include "sampleChannel.h"
#include "recorder.h"


using std::vector;


namespace giada {
namespace m {
namespace recorder
{
namespace
{
/* composite
A group of two actions (keypress+keyrel, muteon+muteoff) used during the overdub
process. */

struct composite
{
	action a1;
	action a2;
} cmp;


/* -------------------------------------------------------------------------- */


/* fixOverdubTruncation
Fixes underlying action truncation when overdubbing over a longer action. I.e.:
  Original:    |#############|
  Overdub:     ---|#######|---
  fix:         |#||#######|--- */

void fixOverdubTruncation(const composite &comp, pthread_mutex_t *mixerMutex)
{
  action *next = nullptr;
  int res = getNextAction(comp.a2.chan, comp.a1.type | comp.a2.type, comp.a2.frame,
    &next);
  if (res != 1 || next->type != comp.a2.type)
    return;
  gu_log("[recorder::fixOverdubTruncation] add truncation at frame %d, type=%d\n",
    next->frame, next->type);
  deleteAction(next->chan, next->frame, next->type, false, mixerMutex);
}

}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


vector<int> frames;
vector<vector<action*>> global;
vector<action*> actions;     // used internally

bool active = false;
bool sortedActions = false;


/* -------------------------------------------------------------------------- */


void init()
{
	active = false;
  sortedActions = false;
	clearAll();
}


/* -------------------------------------------------------------------------- */


bool canRec(Channel *ch, bool clockRunning, bool mixerRecording)
{
	/* NO recording if:
	 * recorder is inactive
	 * mixer is not running
	 * mixer is recording a take somewhere
	 * channel is empty */

	if (!active         ||
		  !clockRunning   ||
			 mixerRecording ||
			(ch->type == CHANNEL_SAMPLE && ((SampleChannel*)ch)->wave == nullptr)
		)
		return false;
	return true;
}


/* -------------------------------------------------------------------------- */


void rec(int index, int type, int frame, uint32_t iValue, float fValue)
{
	/* make sure frame is even */

	if (frame % 2 != 0)
		frame++;

	/* allocating the action */

	action *a = (action*) malloc(sizeof(action));
	a->chan   = index;
	a->type   = type;
	a->frame  = frame;
	a->iValue = iValue;
	a->fValue = fValue;

	/* check if the frame exists in the stack. If it exists, we don't extend
	 * the stack, but we add (or push) a new action to it. */

	int frameToExpand = frames.size();
	for (int i=0; i<frameToExpand; i++)
		if (frames.at(i) == frame) {
			frameToExpand = i;
			break;
		}

	/* espansione dello stack frames nel caso l'azione ricada in frame
	 * non precedentemente memorizzati (frameToExpand == frames.size()).
	 * Espandere frames è facile, basta aggiungere un frame in coda.
	 * Espandere global è più complesso: bisogna prima allocare una
	 * cella in global (per renderlo parallelo a frames) e poi
	 * inizializzare il suo sub-stack (di action). */

	if (frameToExpand == (int) frames.size()) {
		frames.push_back(frame);
		global.push_back(actions);               // array of actions added
		global.at(global.size()-1).push_back(a); // action added
	}
	else {

		/* no duplicates, please */

		for (unsigned t=0; t<global.at(frameToExpand).size(); t++) {
			action *ac = global.at(frameToExpand).at(t);
			if (ac->chan   == index  &&
			    ac->type   == type   &&
			    ac->frame  == frame  &&
			    ac->iValue == iValue &&
			    ac->fValue == fValue)
				return;
		}

		global.at(frameToExpand).push_back(a);		// expand array
	}

	sortedActions = false;

	gu_log("[recorder::rec] action recorded, type=%d frame=%d chan=%d iValue=%d (0x%X) fValue=%f\n",
		a->type, a->frame, a->chan, a->iValue, a->iValue, a->fValue);
	//print();
}


/* -------------------------------------------------------------------------- */


void clearChan(int index)
{
	gu_log("[recorder::clearChan] clearing chan %d...\n", index);

	for (unsigned i=0; i<global.size(); i++) {	// for each frame i
		unsigned j=0;
		while (true) {
			if (j == global.at(i).size()) break; 	  // for each action j of frame i
			action *a = global.at(i).at(j);
			if (a->chan == index)	{
				free(a);
				global.at(i).erase(global.at(i).begin() + j);
			}
			else
				j++;
		}
	}
	optimize();
	//print();
}


/* -------------------------------------------------------------------------- */


void clearAction(int index, char act)
{
	gu_log("[recorder::clearAction] clearing action %d from chan %d...\n", act, index);
	for (unsigned i=0; i<global.size(); i++) {						// for each frame i
		unsigned j=0;
		while (true) {                                   // for each action j of frame i
			if (j == global.at(i).size())
				break;
			action *a = global.at(i).at(j);
			if (a->chan == index && (act & a->type) == a->type)	{ // bitmask
				free(a);
				global.at(i).erase(global.at(i).begin() + j);
			}
			else
				j++;
		}
	}
	optimize();
	//print();
}


/* -------------------------------------------------------------------------- */


void deleteAction(int chan, int frame, char type, bool checkValues,
	pthread_mutex_t *mixerMutex, uint32_t iValue, float fValue)
{
	/* make sure frame is even */

	if (frame % 2 != 0)
		frame++;

	/* find the frame 'frame' */

	bool found = false;
	for (unsigned i=0; i<frames.size() && !found; i++) {

		if (frames.at(i) != frame)
      continue;

			/* find the action in frame i */

		for (unsigned j=0; j<global.at(i).size(); j++) {
			action *a = global.at(i).at(j);

			/* action comparison logic */

			bool doit = (a->chan == chan && a->type == (type & a->type));
			if (checkValues)
				doit &= (a->iValue == iValue && a->fValue == fValue);

      if (!doit)
        continue;

      while (true) {
        if (pthread_mutex_trylock(mixerMutex)) {
          free(a);
          global.at(i).erase(global.at(i).begin() + j);
          pthread_mutex_unlock(mixerMutex);
          found = true;
          break;
        }
        else
          gu_log("[recorder::deleteAction] waiting for mutex...\n");
      }
		}
	}
	if (found) {
		optimize();
		gu_log("[recorder::deleteAction] action deleted, type=%d frame=%d chan=%d iValue=%d (%X) fValue=%f\n",
			type, frame, chan, iValue, iValue, fValue);
	}
	else
		gu_log("[recorder::deleteAction] unable to delete action, not found! type=%d frame=%d chan=%d iValue=%d (%X) fValue=%f\n",
			type, frame, chan, iValue, iValue, fValue);
}


/* -------------------------------------------------------------------------- */


void deleteActions(int chan, int frame_a, int frame_b, char type,
  pthread_mutex_t *mixerMutex)
{
	sortActions();
	vector<int> dels;

	for (unsigned i=0; i<frames.size(); i++)
		if (frames.at(i) > frame_a && frames.at(i) < frame_b)
			dels.push_back(frames.at(i));

	for (unsigned i=0; i<dels.size(); i++)
		deleteAction(chan, dels.at(i), type, false, mixerMutex); // false == don't check values
}


/* -------------------------------------------------------------------------- */


void clearAll()
{
	while (global.size() > 0) {
		for (unsigned i=0; i<global.size(); i++) {
			for (unsigned k=0; k<global.at(i).size(); k++)
				free(global.at(i).at(k));									// free action
			global.at(i).clear();												// free action container
			global.erase(global.begin() + i);
		}
	}
	global.clear();
	frames.clear();
}


/* -------------------------------------------------------------------------- */


void optimize()
{
	/* do something until the i frame is empty. */

	unsigned i = 0;
	while (true) {
		if (i == global.size()) return;
		if (global.at(i).size() == 0) {
			global.erase(global.begin() + i);
			frames.erase(frames.begin() + i);
		}
		else
			i++;
	}

	sortActions();
}


/* -------------------------------------------------------------------------- */


void sortActions()
{
	if (sortedActions)
		return;
	for (unsigned i=0; i<frames.size(); i++)
		for (unsigned j=0; j<frames.size(); j++)
			if (frames.at(j) > frames.at(i)) {
				std::swap(frames.at(j), frames.at(i));
				std::swap(global.at(j), global.at(i));
			}
	sortedActions = true;
	//print();
}


/* -------------------------------------------------------------------------- */


void updateBpm(float oldval, float newval, int oldquanto)
{
	for (unsigned i=0; i<frames.size(); i++) {

		float frame  = ((float) frames.at(i)/newval) * oldval;
		frames.at(i) = (int) frame;

		/* the division up here cannot be precise. A new frame can be 44099
		 * and the quantizer set to 44100. That would mean two recs completely
		 * useless. So we compute a reject value ('scarto'): if it's lower
		 * than 6 frames the new frame is collapsed with a quantized frame. */
		/** XXX - maybe 6 frames are too low */

		if (frames.at(i) != 0) {
			int scarto = oldquanto % frames.at(i);
			if (scarto > 0 && scarto <= 6)
				frames.at(i) = frames.at(i) + scarto;
		}

		/* never ever have odd frames. */

		if (frames.at(i) % 2 != 0)
			frames.at(i)++;
	}

	/* update structs */

	for (unsigned i=0; i<frames.size(); i++) {
		for (unsigned j=0; j<global.at(i).size(); j++) {
			action *a = global.at(i).at(j);
			a->frame = frames.at(i);
		}
	}

	//print();
}


/* -------------------------------------------------------------------------- */


void updateSamplerate(int systemRate, int patchRate)
{
	/* diff ratio: systemRate / patchRate
	 * e.g.  44100 / 96000 = 0.4... */

	if (systemRate == patchRate)
		return;

	gu_log("[recorder::updateSamplerate] systemRate (%d) != patchRate (%d), converting...\n", systemRate, patchRate);

	float ratio = systemRate / (float) patchRate;
	for (unsigned i=0; i<frames.size(); i++) {

		gu_log("[recorder::updateSamplerate]    oldFrame = %d", frames.at(i));

		float newFrame = frames.at(i);
		newFrame = floorf(newFrame * ratio);

		frames.at(i) = (int) newFrame;

		if (frames.at(i) % 2 != 0)
			frames.at(i)++;

		gu_log(", newFrame = %d\n", frames.at(i));
	}

	/* update structs */

	for (unsigned i=0; i<frames.size(); i++) {
		for (unsigned j=0; j<global.at(i).size(); j++) {
			action *a = global.at(i).at(j);
			a->frame = frames.at(i);
		}
	}
}


/* -------------------------------------------------------------------------- */


void expand(int old_fpb, int new_fpb)
{
	/* this algorithm requires multiple passages if we expand from e.g. 2
	 * to 16 beats, precisely 16 / 2 - 1 = 7 times (-1 is the first group,
	 * which exists yet). If we expand by a non-multiple, the result is zero,
	 * due to float->int implicit cast */

	unsigned pass = (int) (new_fpb / old_fpb) - 1;
	if (pass == 0) pass = 1;

	unsigned init_fs = frames.size();

	for (unsigned z=1; z<=pass; z++) {
		for (unsigned i=0; i<init_fs; i++) {
			unsigned newframe = frames.at(i) + (old_fpb*z);
			frames.push_back(newframe);
			global.push_back(actions);
			for (unsigned k=0; k<global.at(i).size(); k++) {
				action *a = global.at(i).at(k);
				rec(a->chan, a->type, newframe, a->iValue, a->fValue);
			}
		}
	}
	gu_log("[recorder::expand] expanded recs\n");
	//print();
}


/* -------------------------------------------------------------------------- */


void shrink(int new_fpb)
{
	/* easier than expand(): here we delete eveything beyond old_framesPerBars. */

	unsigned i=0;
	while (true) {
		if (i == frames.size()) break;

		if (frames.at(i) >= new_fpb) {
			for (unsigned k=0; k<global.at(i).size(); k++)
				free(global.at(i).at(k));		      // free action
			global.at(i).clear();								// free action container
			global.erase(global.begin() + i);   // shrink global
			frames.erase(frames.begin() + i);   // shrink frames
		}
		else
			i++;
	}
	optimize();
	gu_log("[recorder::shrink] shrinked recs\n");
	//print();
}


/* -------------------------------------------------------------------------- */


bool hasActions(int chanIndex)
{
  if (global.size() == 0)
    return false;
	for (unsigned i=0; i<global.size(); i++) {
		for (unsigned j=0; j<global.at(i).size(); j++) {
			if (global.at(i).at(j)->chan == chanIndex)
        return true;
		}
	}
	return false;
}


/* -------------------------------------------------------------------------- */


int getNextAction(int chan, char type, int frame, action **out,
	uint32_t iValue)
{
	sortActions();  // mandatory

	unsigned i=0;
	while (i < frames.size() && frames.at(i) <= frame) i++;

	if (i == frames.size())   // no further actions past 'frame'
		return -1;

	for (; i<global.size(); i++)
		for (unsigned j=0; j<global.at(i).size(); j++) {
			action *a = global.at(i).at(j);
			if (a->chan == chan && (type & a->type) == a->type) {
				//if (iValue == 0 || (iValue != 0 && a->iValue == iValue)) {
				if (iValue == 0 || (iValue != 0 && (iValue & a->iValue) == a->iValue )) {
					*out = global.at(i).at(j);
					return 1;
				}
			}
		}

	return -2;   // no 'type' actions found
}


/* -------------------------------------------------------------------------- */


int getAction(int chan, char action, int frame, struct action **out)
{
	for (unsigned i=0; i<global.size(); i++)
		for (unsigned j=0; j<global.at(i).size(); j++)
			if (frame  == global.at(i).at(j)->frame &&
					action == global.at(i).at(j)->type &&
					chan   == global.at(i).at(j)->chan)
			{
				*out = global.at(i).at(j);
				return 1;
			}
	return 0;
}


/* -------------------------------------------------------------------------- */


void startOverdub(int index, char actionMask, int frame, unsigned bufferSize)
{
	/* prepare the composite struct */

	if (actionMask == G_ACTION_KEYS) {
		cmp.a1.type = G_ACTION_KEYPRESS;
		cmp.a2.type = G_ACTION_KEYREL;
	}
	else {
		cmp.a1.type = G_ACTION_MUTEON;
		cmp.a2.type = G_ACTION_MUTEOFF;
	}
	cmp.a1.chan  = index;
	cmp.a2.chan  = index;
	cmp.a1.frame = frame;
	// cmp.a2.frame doesn't exist yet

	/* avoid underlying action truncation: if action2.type == nextAction:
	 * you are in the middle of a composite action, truncation needed */

	rec(index, cmp.a1.type, frame);

	action *act = nullptr;
	int res = getNextAction(index, cmp.a1.type | cmp.a2.type, cmp.a1.frame, &act);
	if (res == 1) {
		if (act->type == cmp.a2.type) {
			int truncFrame = cmp.a1.frame - bufferSize;
			if (truncFrame < 0)
				truncFrame = 0;
			gu_log("[recorder::startOverdub] add truncation at frame %d, type=%d\n", truncFrame, cmp.a2.type);
			rec(index, cmp.a2.type, truncFrame);
		}
	}
}


/* -------------------------------------------------------------------------- */


void stopOverdub(int currentFrame, int totalFrames, pthread_mutex_t *mixerMutex)
{
	cmp.a2.frame  = currentFrame;
	bool ringLoop = false;
	bool nullLoop = false;

	/* Check for ring loops or null loops. Ring loop: a composite action with
  key_press at frame N and key_release at frame M, with M <= N.
  Null loop: a composite action that begins and ends on the very same frame,
  i.e. with 0 size. Very unlikely.
  If ring loop: record the last action at the end of the sequencer (that
  is 'totalFrames').
  If null loop: remove previous action and do nothing. Also make sure to avoid
  underlying action truncation, if the null loop occurs inside a composite
  action. */

	if (cmp.a2.frame < cmp.a1.frame) {  // ring loop
		ringLoop = true;
		gu_log("[recorder::stopOverdub] ring loop! frame1=%d < frame2=%d\n", cmp.a1.frame, cmp.a2.frame);
		rec(cmp.a2.chan, cmp.a2.type, totalFrames);
	}
	else
	if (cmp.a2.frame == cmp.a1.frame) { // null loop
		nullLoop = true;
		gu_log("[recorder::stopOverdub] null loop! frame1=%d == frame2=%d\n", cmp.a1.frame, cmp.a2.frame);
		deleteAction(cmp.a1.chan, cmp.a1.frame, cmp.a1.type, false, mixerMutex); // false == don't check values
    fixOverdubTruncation(cmp, mixerMutex);
  }

  if (nullLoop)
    return;

	/* Remove any nested action between keypress----keyrel. */

	deleteActions(cmp.a2.chan, cmp.a1.frame, cmp.a2.frame, cmp.a1.type, mixerMutex);
	deleteActions(cmp.a2.chan, cmp.a1.frame, cmp.a2.frame, cmp.a2.type, mixerMutex);

  if (ringLoop)
    return;

  /* Record second part of the composite action. Also make sure to avoid
  underlying action truncation, if keyrel happens inside a composite action. */

	rec(cmp.a2.chan, cmp.a2.type, cmp.a2.frame);
  fixOverdubTruncation(cmp, mixerMutex);
}


}}}; // giada::m::recorder::
