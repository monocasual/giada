/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * glue
 * Intermediate layer GUI <-> CORE.
 *
 * How to know if you need another glue_ function? Ask yourself if the
 * new action will ever be called via MIDI or keyboard/mouse. If yes,
 * put it here.
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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
#include "../gui/elems/ge_keyboard.h"
#include "../gui/elems/channel.h"
#include "../gui/elems/sampleChannel.h"
#include "../utils/gui.h"
#include "../utils/log.h"
#include "../core/recorder.h"
#include "../core/mixer.h"
#include "../core/mixerHandler.h"
#include "../core/wave.h"
#include "../core/channel.h"
#include "../core/sampleChannel.h"
#include "../core/midiChannel.h"
#include "glue.h"
#include "io.h"


extern Recorder			 G_Recorder;
extern bool 		 		 G_audio_status;
extern Mixer	   		 G_Mixer;
extern gdMainWindow *G_MainWin;


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
		ch->start(0, true, G_Mixer.quantize, G_Mixer.running); // on frame 0: user-generated event
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
		if (G_Recorder.active) {
			if (G_Mixer.running) {
				ch->kill(0); // on frame 0: user-generated event
				if (G_Recorder.canRec(ch) && !(ch->mode & LOOP_ANY))   // don't record killChan actions for LOOP channels
					G_Recorder.rec(ch->index, ACTION_KILLCHAN, G_Mixer.actualFrame);
			}
		}
		else {
			if (ch->hasActions) {
				if (G_Mixer.running || ch->status == STATUS_OFF)
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

		if (G_Mixer.quantize == 0  &&
		    G_Recorder.canRec(ch)  &&
	      !(ch->mode & LOOP_ANY))
		{
			if (ch->mode == SINGLE_PRESS)
				G_Recorder.startOverdub(ch->index, ACTION_KEYS, G_Mixer.actualFrame);
			else {

				/* Why return here? You record an action (as done on line 156) and then
				you call ch->start: Mixer, which is on another thread, reads your newly
				recorded action, and then ch->start kicks in right after it	(as done on
				line 163).
				The result: Mixer plays the channel (due to the new action) but ch->start
				kills it right away (because the sample is playing). Fix: call ch->start
				only if you are not recording anything, i.e. let Mixer play it. */

				G_Recorder.rec(ch->index, ACTION_KEYPRESS, G_Mixer.actualFrame);
				return;
			}
		}

		/* This is a user-generated event, so it's on frame 0 */

		ch->start(0, true, G_Mixer.quantize, G_Mixer.running, true);
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

	if (ch->mode == SINGLE_PRESS && G_Recorder.canRec(ch))
		G_Recorder.stopOverdub(G_Mixer.actualFrame);

	/* the GUI update is done by gui_refresh() */

}


/* -------------------------------------------------------------------------- */


void glue_startStopActionRec()
{
	G_Recorder.active ? glue_stopActionRec() : glue_startActionRec();
}


/* -------------------------------------------------------------------------- */


void glue_startActionRec()
{
	if (G_audio_status == false)
		return;
	G_Recorder.active = true;

	Fl::lock();
	G_MainWin->controller->updateRecAction(1);
	Fl::unlock();
}


/* -------------------------------------------------------------------------- */


void glue_stopActionRec()
{
	/* stop the recorder and sort new actions */

	G_Recorder.active = false;
	G_Recorder.sortActions();

	for (unsigned i=0; i<G_Mixer.channels.size(); i++)
	{
		if (G_Mixer.channels.at(i)->type == CHANNEL_MIDI)
			continue;
		SampleChannel *ch = (SampleChannel*) G_Mixer.channels.at(i);
		G_MainWin->keyboard->setChannelWithActions((geSampleChannel*)ch->guiChannel);
		if (!ch->readActions && ch->hasActions)
			glue_startReadingRecs(ch, false);
	}

	Fl::lock();
	G_MainWin->controller->updateRecAction(0);
	Fl::unlock();

	/* in case acton editor is on, refresh it */

	gu_refreshActionEditor();
}


/* -------------------------------------------------------------------------- */


void glue_startStopInputRec(bool gui, bool alert)
{
	if (G_Mixer.chanInput == NULL) {
		if (!glue_startInputRec(gui)) {
			if (alert) gdAlert("No channels available for recording.");
			else       gu_log("[glue] no channels available for recording\n");
		}
	}
	else
		glue_stopInputRec(gui);
}


/* -------------------------------------------------------------------------- */


int glue_startInputRec(bool gui)
{
	if (G_audio_status == false)
		return -1;

	SampleChannel *ch = mh_startInputRec();
	if (ch == NULL)	{                  // no chans available
		Fl::lock();
		G_MainWin->controller->updateRecInput(0);
		Fl::unlock();
		return 0;
	}

	if (!G_Mixer.running) {
		glue_startSeq();
		Fl::lock();
		G_MainWin->controller->updatePlay(1);
		Fl::unlock();
	}

	glue_setChanVol(ch, 1.0f, false); // false = not from gui click

	ch->guiChannel->mainButton->label(ch->wave->name.c_str());

	if (!gui) {
		Fl::lock();
		G_MainWin->controller->updateRecInput(1);
		Fl::unlock();
	}

	return 1;

}


/* -------------------------------------------------------------------------- */


int glue_stopInputRec(bool gui)
{
	SampleChannel *ch = mh_stopInputRec();

	if (ch->mode & (LOOP_BASIC | LOOP_ONCE | LOOP_REPEAT))
		ch->start(0, true, G_Mixer.quantize, G_Mixer.running);  // on frame 0: user-generated event

	if (!gui) {
		Fl::lock();
		G_MainWin->controller->updateRecInput(0);
		Fl::unlock();
	}

	return 1;
}
