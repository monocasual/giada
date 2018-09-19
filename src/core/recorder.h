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


#ifndef G_RECORDER_H
#define G_RECORDER_H


#include <cstdint>
#include <vector>
#include <functional>
#include <pthread.h>


class Channel;


namespace giada {
namespace m {
namespace recorder
{
/* action
 * struct containing fields to describe an atomic action. Note from
 * VST sdk: parameter values, like all VST parameters, are declared as
 * floats with an inclusive range of 0.0 to 1.0 (fValue). */

struct action
{
	int      chan;    // channel index, i.e. Channel->index
 	int      type;
	int      frame;   // redundant info, used by helper functions
	float    fValue;  // used only for envelopes (volumes, vst params).
	uint32_t iValue;  // used only for MIDI events
};

/* Composite
A group of two actions (keypress+keyrel, muteon+muteoff). */

struct Composite
{
	action a1;
	action a2;
};

/* frames
Frame counter sentinel. It tells which frames contain actions. E.g.:
  frames[0] = 155   // some actions on frame 155
  frames[1] = 2048  // some actions on frame 2048
It always matches 'global''s size: frames.size() == global.size() */

extern std::vector<int> frames;

/* global
Contains the actual actions. E.g.:
  global[0] = <actions>
  global[1] = <actions> */

extern std::vector<std::vector<action*>> global;
/* TODO - this frames vs global madness must be replaced with a map:
std::map<int, vector<actions>> */

extern bool active;
extern bool sortedActions;   // are actions sorted via sortActions()?

/* init
 * everything starts from here. */

void init();

/* hasActions
Checks if the channel has at least one action recorded. Used after an
action deletion. Type != -1: check if channel has actions of type 'type'.*/

bool hasActions(int chanIndex, int type=-1);

/* canRec
 * can a channel rec an action? Call this one BEFORE rec(). */

bool canRec(Channel* ch, bool clockRunning, bool mixerRecording);

/* rec
 * record an action. */

void rec(int chan, int action, int frame, uint32_t iValue=0, float fValue=0.0f);

/* clearChan
 * clear all actions from a channel. */

void clearChan(int chan);

/* clearAction
 * clear the 'action' action type from a channel. */

void clearAction(int chan, char action);

/* deleteAction
 * delete ONE action. Useful in the action editor. 'type' can be a mask. */

void deleteAction(int chan, int frame, char type, bool checkValues,
  pthread_mutex_t* mixerMutex, uint32_t iValue=0, float fValue=0.0);

/* deleteActions
Deletes A RANGE of actions from frame_a to frame_b in channel 'chan' of type
'type' (can be a bitmask). Exclusive range (frame_a, frame_b). */

void deleteActions(int chan, int frame_a, int frame_b, char type,
  pthread_mutex_t* mixerMutex);

/* clearAll
 * delete everything. */

void clearAll();

/* optimize
 * clear frames without actions. */

void optimize();

/* sortActions
 * sorts actions by frame, asc mode. */

void sortActions();

/* updateBpm
 * reassign frames by calculating the new bpm value. */

void updateBpm(float oldval, float newval, int oldquanto);

/* updateSamplerate
 * reassign frames taking in account the samplerate. If f_system ==
 * f_patch nothing changes, otherwise the conversion is mandatory. */

void updateSamplerate(int systemRate, int patchRate);

void expand(int old_fpb, int new_fpb);
void shrink(int new_fpb);

/* getNextAction
Returns the nearest action in chan 'chan' of type 'action' starting from 
'frame'. Action can be a bitmask. If iValue != 0 search for next action with 
iValue == iValue with 'mask' to ignore bytes. Useful for MIDI key_release. 
Mask example:

	iValue = 0x803D3F00
	mask   = 0x0000FF00  // ignore byte 3
	action = 0x803D3200  // <--- this action will be found */

int getNextAction(int chan, char action, int frame, struct action** out,
	uint32_t iValue=0, uint32_t mask=0);

/* getAction
Returns a pointer to action in chan 'chan' of type 'action' at frame 'frame'. */

int getAction(int chan, char action, int frame, struct action** out);

/* getActionsOnFrame
Returns a vector of actions that occur on frame 'frame'. */

std::vector<action*> getActionsOnFrame(int frame);

/* start/stopOverdub
These functions are used when you overwrite existing actions. For example:
pressing Mute button on a channel with some existing mute actions. */

void startOverdub(int chan, char action, int frame, unsigned bufferSize);
void stopOverdub(int currentFrame, int totalFrames, pthread_mutex_t* mixerMutex);

/* forEachAction
Applies a read-only callback on each action recorded. */

void forEachAction(std::function<void(const action*)> f);
}}}; // giada::m::recorder::

#endif
