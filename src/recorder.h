/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * recorder
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

#ifndef RECORDER_H
#define RECORDER_H

#include <stdio.h>
#include <stdlib.h>
#include "const.h"
#include "utils.h"
#include "mixer.h"

/*
 * [global0]-->[gVector<_action*>0]-->[a0][a1][a2]				0[frames1]
 * [global1]-->[gVector<_action*>1]-->[a0][a1][a2]				1[frames2]
 * [global2]-->[gVector<_action*>2]-->[a0][a1][a2]				2[frames3]
 * [global3]-->[gVector<_action*>3]-->[a0][a1][a2]				3[frames4]
 * */

namespace recorder {

struct _action { /** FIXME - please remove the underscore */
	int  chan;
	char type;
};

extern gVector<int>  frames;					      // frame counter (sentinel) frames.size == global.size
extern gVector< gVector<_action*> > global;	// container of containers of actions
extern gVector<_action*>  actions;				  // container of actions

extern bool active;
extern bool chanActive[MAX_NUM_CHAN];				// recs are read only for active channels
extern bool chanEvents[MAX_NUM_CHAN];				// chan has events?
extern bool sortedActions;                  // actions are sorted via sortActions()?

/* init
 * everything starts from here. */
void init();

/* chanHasEvents
 * Checks if the channel has at least one action recorded. If false, sets
 * chanEvents[ch] = false. Used after an action deletion. */
void chanHasEvents(int chan);

/* canRec
 * can we rec an action? Call this one BEFORE rec(). */
bool canRec(int chan);

/* rec
 * records an action. */
void rec(int chan, char action, int frame);

/* clearChan
 * clears all actions from a channel. */
void clearChan(int chan);

/* clearAction
 * clears the 'action' action type from a channel. */
void clearAction(int chan, char action);

/* deleteAction
 * deletes ONE action. Useful in the action editor. */
void deleteAction(int chan, int frame, char type);

/* clearAll
 * deletes everything. */
void clearAll();

/* optimize
 * clears frames without actions. */
void optimize();

/* sortActions
 * sorts actions, asc mode. */
void sortActions();

/* updateBpm
 * reassigns frames by calculating the new bpm value. */
void updateBpm(float oldval, float newval, int oldquanto);

/* updateSamplerate
 * reassigns frames taking in account the samplerate. If f_system ==
 * f_patch nothing changes, otherwise the conversion is mandatory. */
void updateSamplerate(int systemRate, int patchRate);

void expand(int old_fpb, int new_fpb);
void shrink(int new_fpb);

/* enable/disableRead
 * if enabled  = read actions from channel chan
 * if disabled = don't read actions from channel chan. */
void enableRead(int chan);
void disableRead(int chan);

/* getStartActionFrame
 * searches for the A-frame of a pair of actions, e.g. MUTE_OFF(a) +
 * MUTE_ON(b). Returns the MUTE_OFF frame, if any. 'action' is the
 * action to look for. */
int getStartActionFrame(int chan, char action, int frame);

/* getEndActionFrame
 * same as getStartActionFrame but searches the B-frame. */
int getEndActionFrame(int chan, char action, int frame);

/* print
 * debug of the frame stack. */
void print();

}  // namespace

#endif
