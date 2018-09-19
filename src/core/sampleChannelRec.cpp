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


#include <cassert>
#include "const.h"
#include "conf.h"
#include "clock.h"
#include "kernelAudio.h"
#include "sampleChannel.h"
#include "sampleChannelRec.h"


namespace giada {
namespace m {
namespace sampleChannelRec
{
namespace
{
/* onFirstBeat
Things to do when the sequencer is on the first beat. */

void onFirstBeat_(SampleChannel* ch, bool recsStopOnChanHalt)
{
	if (ch->wave == nullptr)
		return;

	switch (ch->recStatus) {
		case ChannelStatus::ENDING:
			ch->recStatus = ChannelStatus::OFF;
			setReadActions(ch, false, recsStopOnChanHalt);  // rec stop
			break;

		case ChannelStatus::WAIT:
			ch->recStatus = ChannelStatus::PLAY;
			setReadActions(ch, true, recsStopOnChanHalt);   // rec start			
			break;

		default: break;
	}
}


/* -------------------------------------------------------------------------- */


bool recorderCanRec_(SampleChannel* ch)
{
	return recorder::canRec(ch, clock::isRunning(), mixer::recording);
}


/* -------------------------------------------------------------------------- */


/* calcVolumeEnv
Computes any changes in volume done via envelope tool. */

void calcVolumeEnv_(SampleChannel* ch, int globalFrame)
{
	/* method: check this frame && next frame, then calculate delta */

	recorder::action* a0 = nullptr;
	recorder::action* a1 = nullptr;
	int res;

	/* get this action on frame 'frame'. It's unlikely that the action
	 * is not found. */

	res = recorder::getAction(ch->index, G_ACTION_VOLUME, globalFrame, &a0);

	assert(res != 0);

	/* get the action next to this one.
	 * res == -1: a1 not found, this is the last one. Rewind the search
	 * and use action at frame number 0 (actions[0]).
	 * res == -2 G_ACTION_VOLUME not found. This should never happen */

	res = recorder::getNextAction(ch->index, G_ACTION_VOLUME, globalFrame, &a1);
	if (res == -1)
		res = recorder::getAction(ch->index, G_ACTION_VOLUME, 0, &a1);

	assert(res != -2);

	ch->volume_i = a0->fValue;
	ch->volume_d = ((a1->fValue - a0->fValue) / (a1->frame - a0->frame)) * 1.003f;
}


/* -------------------------------------------------------------------------- */


void parseAction_(SampleChannel* ch, const recorder::action* a, int localFrame, 
	int globalFrame)
{
	if (!ch->readActions)
		return;

	switch (a->type) {
		case G_ACTION_KEYPRESS:
			if (ch->isAnySingleMode()) {
				ch->start(localFrame, false, 0);
				/* This is not a user-generated event, so fill the first chunk of buffer. 
				Then, sampleChannelProc::prepareBuffer will take care of filling the
				subsequent buffers from the next cycle on. */
				if (ch->status == ChannelStatus::PLAY)
					ch->tracker += ch->fillBuffer(ch->buffer, ch->tracker, localFrame);
			}
			break;
		case G_ACTION_KEYREL:
			if (ch->isAnySingleMode())
				ch->stop();
			break;
		case G_ACTION_KILL:
			if (ch->isAnySingleMode())
				ch->kill(localFrame);
			break;
		case G_ACTION_VOLUME:
			calcVolumeEnv_(ch, globalFrame);
			break;
	}
}


/* -------------------------------------------------------------------------- */


void recordKeyPressAction_(SampleChannel* ch)
{
	if (!recorderCanRec_(ch))
		return;

	/* SINGLE_PRESS mode needs overdub. Also, disable reading actions while 
	overdubbing. */
	if (ch->mode == ChannelMode::SINGLE_PRESS) {
		recorder::startOverdub(ch->index, G_ACTION_KEYS, clock::getCurrentFrame(), 
			kernelAudio::getRealBufSize());
		ch->readActions = false;
	}
	else
		recorder::rec(ch->index, G_ACTION_KEYPRESS, clock::getCurrentFrame());
	ch->hasActions = true;
}


/* -------------------------------------------------------------------------- */


void quantize_(SampleChannel* ch, bool quantoPassed)
{
	/* Skip if LOOP_ANY or not in quantizer-wait mode. Otherwise the quantize wait 
	has expired: record the keypress.  */

	if (ch->isAnyLoopMode() || !ch->qWait || !quantoPassed)
		return;
	recordKeyPressAction_(ch);
}
}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void parseEvents(SampleChannel* ch, mixer::FrameEvents fe)
{
	quantize_(ch, fe.quantoPassed);
	if (fe.onFirstBeat)
		onFirstBeat_(ch, conf::recsStopOnChanHalt);
	for (const recorder::action* action : fe.actions)
		if (action->chan == ch->index)
			parseAction_(ch, action, fe.frameLocal, fe.frameGlobal);
}


/* -------------------------------------------------------------------------- */


bool recordStart(SampleChannel* ch, bool canQuantize)
{
	/* Record a 'start' event if the quantizer is off, otherwise let mixer to 
	handle it when a quantoWait has passed (see quantize_()). Also skip if 
	channel is in any loop mode, where KEYPRESS and KEYREL are meaningless. */

	if (!canQuantize && !ch->isAnyLoopMode() && recorderCanRec_(ch))
	{
		recordKeyPressAction_(ch);

		/* Why return here? You record an action and then you call ch->start: 
		Mixer, which is on another thread, reads your newly recorded action if you 
		have readActions == true, and then ch->start kicks in right after it.
		The result: Mixer plays the channel (due to the new action) but the code
		in the switch in start() kills it right away (because the sample is playing). 
		Fix: start channel only if you are not recording anything, i.e. let 
		Mixer play it. */

		if (ch->readActions)
			return false;
	}
	return true;
}


/* -------------------------------------------------------------------------- */


bool recordKill(SampleChannel* ch)
{
	/* Don't record G_ACTION_KILL actions for LOOP channels. */
	if (recorderCanRec_(ch) && !ch->isAnyLoopMode()) {
		recorder::rec(ch->index, G_ACTION_KILL, clock::getCurrentFrame());
		ch->hasActions = true;
	}
	return true;
}


/* -------------------------------------------------------------------------- */


void recordStop(SampleChannel* ch)
{
	/* Record a stop event only if channel is SINGLE_PRESS. For any other mode 
	the stop event is meaningless. */
	if (recorderCanRec_(ch) && ch->mode == ChannelMode::SINGLE_PRESS) {
		recorder::stopOverdub(clock::getCurrentFrame(), clock::getFramesInLoop(),
			&mixer::mutex);
	}
}


/* -------------------------------------------------------------------------- */


void setReadActions(SampleChannel* ch, bool v, bool recsStopOnChanHalt)
{
	ch->readActions = v;
	if (!ch->readActions && recsStopOnChanHalt)
		ch->kill(0); // FIXME - wrong frame value
}


/* -------------------------------------------------------------------------- */


void startReadingActions(SampleChannel* ch, bool treatRecsAsLoops, bool recsStopOnChanHalt)
{
	if (treatRecsAsLoops)
		ch->recStatus = ChannelStatus::WAIT;
	else
		setReadActions(ch, true, recsStopOnChanHalt);
}


/* -------------------------------------------------------------------------- */


void stopReadingActions(SampleChannel* ch, bool isClockRunning, bool treatRecsAsLoops, 
		bool recsStopOnChanHalt)
{
	/* First of all, if the clock is not running just stop and disable everything.
	Then if "treatRecsAsLoop" wait until the sequencer reaches beat 0, so put the
	channel in REC_ENDING status. */

	if (!isClockRunning) {
		ch->recStatus = ChannelStatus::OFF;
		setReadActions(ch, false, false);
	}
	else
	if (ch->recStatus == ChannelStatus::WAIT)
		ch->recStatus = ChannelStatus::OFF;
	else
	if (ch->recStatus == ChannelStatus::ENDING)
		ch->recStatus = ChannelStatus::PLAY;
	else
	if (treatRecsAsLoops)
		ch->recStatus = ChannelStatus::ENDING;
	else
		setReadActions(ch, false, recsStopOnChanHalt);
}
}}};