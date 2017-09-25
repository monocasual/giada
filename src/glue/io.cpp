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


#include <FL/Fl.H>
#include "../gui/dialogs/gd_mainWindow.h"
#include "../gui/dialogs/gd_warnings.h"
#include "../gui/elems/mainWindow/mainTransport.h"
#include "../gui/elems/mainWindow/mainTimer.h"
#include "../gui/elems/mainWindow/keyboard/keyboard.h"
#include "../gui/elems/mainWindow/keyboard/channel.h"
#include "../gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "../utils/gui.h"
#include "../utils/log.h"
#include "../core/recorder.h"
#include "../core/kernelAudio.h"
#include "../core/mixer.h"
#include "../core/mixerHandler.h"
#include "../core/wave.h"
#include "../core/channel.h"
#include "../core/clock.h"
#include "../core/sampleChannel.h"
#include "../core/midiChannel.h"
#include "main.h"
#include "channel.h"
#include "transport.h"
#include "io.h"


extern gdMainWindow *G_MainWin;


using namespace giada::m;


void glue_keyPress(Channel *ch, bool ctrl, bool shift)
{
	if (ch->type == CHANNEL_SAMPLE)
		glue_keyPress((SampleChannel*)ch, ctrl, shift);
	else
		glue_keyPress((MidiChannel*)ch, ctrl, shift);
}


/* -------------------------------------------------------------------------- */


void glue_keyRelease(Channel *ch, bool ctrl, bool shift)
{
	if (ch->type == CHANNEL_SAMPLE)
		glue_keyRelease((SampleChannel*)ch, ctrl, shift);
}


/* -------------------------------------------------------------------------- */


void glue_keyPress(MidiChannel *ch, bool ctrl, bool shift)
{
	if (ctrl)
		glue_setMute(ch);
	else
	if (shift)
		ch->kill(0);        // on frame 0: user-generated event
	else
		ch->start(0, true, clock::getQuantize(), clock::isRunning(), false, true); // on frame 0: user-generated event
}


/* -------------------------------------------------------------------------- */


void glue_keyPress(SampleChannel *ch, bool ctrl, bool shift)
{
	/* case CTRL */

	if (ctrl)
		glue_setMute(ch);

	/* case SHIFT
	 *
	 * action recording on:
	 * 		if seq is playing, rec a killchan
	 * action recording off:
	 * 		if chan has recorded events:
	 * 		|	 if seq is playing OR channel 'c' is stopped, de/activate recs
	 * 		|	 else kill chan
	 *		else kill chan */

	else
	if (shift) {
		if (recorder::active) {
			if (clock::isRunning()) {
				ch->kill(0); // on frame 0: user-generated event
				if (recorder::canRec(ch, clock::isRunning(), mixer::recording) &&
            !(ch->mode & LOOP_ANY))
        {   // don't record killChan actions for LOOP channels
					recorder::rec(ch->index, G_ACTION_KILL, clock::getCurrentFrame());
          ch->hasActions = true;
        }
			}
		}
		else {
			if (ch->hasActions) {
				if (clock::isRunning() || ch->status == STATUS_OFF)
					ch->readActions ? glue_stopReadingRecs(ch) : glue_startReadingRecs(ch);
				else
					ch->kill(0);  // on frame 0: user-generated event
			}
			else
				ch->kill(0);    // on frame 0: user-generated event
		}
	}
	else {  /* case no modifier */

		/* record now if the quantizer is off, otherwise let mixer to handle it
		 * when a quantoWait has passed. Moreover, KEYPRESS and KEYREL are
		 * meaningless for loop modes */

		if (clock::getQuantize() == 0 &&
		    recorder::canRec(ch, clock::isRunning(), mixer::recording) &&
	      !(ch->mode & LOOP_ANY))
		{
			if (ch->mode == SINGLE_PRESS) {
				recorder::startOverdub(ch->index, G_ACTION_KEYS, clock::getCurrentFrame(),
          kernelAudio::getRealBufSize());
        ch->readActions = false;   // don't read actions while overdubbing
      }
			else {
				recorder::rec(ch->index, G_ACTION_KEYPRESS, clock::getCurrentFrame());
        ch->hasActions = true;

        /* Why return here? You record an action (as done on line 148) and then
				you call ch->start (line 165): Mixer, which is on another thread, reads
        your newly recorded action if you have readActions == true, and then
        ch->start kicks in right after it	(as done on	line 165).
				The result: Mixer plays the channel (due to the new action) but ch->start
				kills it right away (because the sample is playing). Fix: call ch->start
				only if you are not recording anything, i.e. let Mixer play it. */

        if (ch->readActions)
          return;
			}
		}

		/* This is a user-generated event, so it's on frame 0 */

		ch->start(0, true, clock::getQuantize(), clock::isRunning(), false, true);
	}

	/* the GUI update is done by gui_refresh() */
}


/* -------------------------------------------------------------------------- */


void glue_keyRelease(SampleChannel *ch, bool ctrl, bool shift)
{
	if (ctrl || shift)
		return;

	ch->stop();

	/* record a key release only if channel is single_press. For any
	 * other mode the KEY REL is meaningless. */

	if (ch->mode == SINGLE_PRESS && recorder::canRec(ch, clock::isRunning(), mixer::recording))
		recorder::stopOverdub(clock::getCurrentFrame(), clock::getTotalFrames(),
      &mixer::mutex_recs);

	/* the GUI update is done by gui_refresh() */

}


/* -------------------------------------------------------------------------- */


void glue_startStopActionRec(bool gui)
{
	recorder::active ? glue_stopActionRec(gui) : glue_startActionRec(gui);
}


/* -------------------------------------------------------------------------- */


void glue_startActionRec(bool gui)
{
	if (kernelAudio::getStatus() == false)
		return;

	recorder::active = true;

	if (!clock::isRunning())
		glue_startSeq(false);  // update gui ayway

	if (!gui) {
		Fl::lock();
		G_MainWin->mainTransport->updateRecAction(1);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void glue_stopActionRec(bool gui)
{
	/* stop the recorder and sort new actions */

	recorder::active = false;
	recorder::sortActions();

	for (unsigned i=0; i<mixer::channels.size(); i++)
	{
		if (mixer::channels.at(i)->type == CHANNEL_MIDI)
			continue;
		SampleChannel *ch = (SampleChannel*) mixer::channels.at(i);
		G_MainWin->keyboard->setChannelWithActions((geSampleChannel*)ch->guiChannel);
		if (!ch->readActions && ch->hasActions)
			glue_startReadingRecs(ch, false);
	}

	if (!gui) {
		Fl::lock();
		G_MainWin->mainTransport->updateRecAction(0);
		Fl::unlock();
	}

	gu_refreshActionEditor();  // in case it's open
}


/* -------------------------------------------------------------------------- */


void glue_startStopInputRec(bool gui)
{
	if (mixer::recording)
		glue_stopInputRec(gui);
	else
	if (!glue_startInputRec(gui))
		gdAlert("No channels armed/available for audio recording.");
}


/* -------------------------------------------------------------------------- */


int glue_startInputRec(bool gui)
{
	if (kernelAudio::getStatus() == false)
		return false;

	if (!mh::startInputRec()) {
	  Fl::lock();
	  G_MainWin->mainTransport->updateRecInput(0);  // set it off, anyway
		Fl::unlock();
		return false;
	}

	if (!clock::isRunning())
		glue_startSeq(false); // update gui anyway

  Fl::lock();
    if (!gui)
		  G_MainWin->mainTransport->updateRecInput(1);
    G_MainWin->mainTimer->setLock(true);
  Fl::unlock();

  /* Update sample name inside sample channels' main button. This is useless for
  midi channel, but let's do it anyway. */

  for (unsigned i=0; i<mixer::channels.size(); i++)
    mixer::channels.at(i)->guiChannel->update();

	return true;
}


/* -------------------------------------------------------------------------- */


int glue_stopInputRec(bool gui)
{
	mh::stopInputRec();

	/* Start all sample channels in loop mode that were armed, i.e. that were
	recording stuff and not yet in play. They are also started in force mode, i.e.
  they must start playing right away at the current frame, not at the next first
  beat. */

	for (unsigned i=0; i<mixer::channels.size(); i++) {
		if (mixer::channels.at(i)->type == CHANNEL_MIDI)
			continue;
		SampleChannel *ch = (SampleChannel*) mixer::channels.at(i);
		if (ch->mode & (LOOP_ANY) && ch->status == STATUS_OFF && ch->isArmed())
			ch->start(clock::getCurrentFrame(), true, clock::getQuantize(),
        clock::isRunning(), true, true);
	}

  Fl::lock();
    if (!gui)
		  G_MainWin->mainTransport->updateRecInput(0);
    G_MainWin->mainTimer->setLock(false);
  Fl::unlock();

	return 1;
}
