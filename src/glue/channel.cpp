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


#include <cmath>
#include "../gui/dialogs/gd_mainWindow.h"
#include "../gui/dialogs/sampleEditor.h"
#include "../gui/elems/basics/input.h"
#include "../gui/elems/basics/dial.h"
#include "../gui/elems/sampleEditor/waveTools.h"
#include "../gui/elems/sampleEditor/waveform.h"
#include "../gui/elems/mainWindow/keyboard/keyboard.h"
#include "../gui/elems/mainWindow/keyboard/channel.h"
#include "../gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "../gui/elems/mainWindow/keyboard/channelButton.h"
#include "../utils/gui.h"
#include "../utils/fs.h"
#include "../core/kernelAudio.h"
#include "../core/mixerHandler.h"
#include "../core/mixer.h"
#include "../core/clock.h"
#include "../core/pluginHost.h"
#include "../core/conf.h"
#include "../core/wave.h"
#include "../core/channel.h"
#include "../core/sampleChannel.h"
#include "../core/midiChannel.h"
#include "../core/plugin.h"
#include "main.h"
#include "channel.h"


extern gdMainWindow *G_MainWin;


using std::string;
using namespace giada;


static bool __soloSession__ = false;


int glue_loadChannel(SampleChannel *ch, const string &fname)
{
  /* Always stop a channel before loading a new sample in it. This will prevent
  issues if tracker is outside the boundaries of the new sample -> segfault. */

  ch->hardStop(0);

	/* save the patch and take the last browser's dir in order to re-use it
	 * the next time */

	conf::samplePath = gu_dirname(fname);

	int result = ch->load(fname.c_str(), conf::samplerate, conf::rsmpQuality);

	if (result == SAMPLE_LOADED_OK)
		G_MainWin->keyboard->updateChannel(ch->guiChannel);

	return result;
}


/* -------------------------------------------------------------------------- */


Channel *glue_addChannel(int column, int type)
{
	Channel *ch     = mh::addChannel(type);
	geChannel *gch  = G_MainWin->keyboard->addChannel(column, ch);
	ch->guiChannel  = gch;
	return ch;
}


/* -------------------------------------------------------------------------- */


void glue_deleteChannel(Channel *ch)
{
	recorder::clearChan(ch->index);
  ch->hasActions = false;
#ifdef WITH_VST
	pluginHost::freeStack(pluginHost::CHANNEL, &mixer::mutex_plugins, ch);
#endif
	Fl::lock();
	G_MainWin->keyboard->deleteChannel(ch->guiChannel);
	Fl::unlock();
	mh::deleteChannel(ch);
	gu_closeAllSubwindows();
}


/* -------------------------------------------------------------------------- */


void glue_freeChannel(Channel *ch)
{
	G_MainWin->keyboard->freeChannel(ch->guiChannel);
	recorder::clearChan(ch->index);
  ch->hasActions = false;
	ch->empty();
}


/* -------------------------------------------------------------------------- */


void glue_toggleArm(Channel *ch, bool gui)
{
	ch->armed = !ch->armed;
	if (!gui)
		ch->guiChannel->arm->value(ch->armed);
}


/* -------------------------------------------------------------------------- */


int glue_cloneChannel(Channel *src)
{
	Channel *ch    = mh::addChannel(src->type);
	geChannel *gch = G_MainWin->keyboard->addChannel(src->guiChannel->getColumnIndex(), ch);

	ch->guiChannel = gch;
	ch->copy(src, &mixer::mutex_plugins);

	G_MainWin->keyboard->updateChannel(ch->guiChannel);
	return true;
}


/* -------------------------------------------------------------------------- */


void glue_setChanVol(Channel *ch, float v, bool gui)
{
	ch->volume = v;

	/* also update wave editor if it's shown */

	gdSampleEditor *editor = (gdSampleEditor*) gu_getSubwindow(G_MainWin, WID_SAMPLE_EDITOR);
	if (editor) {
		glue_setVolEditor(editor, (SampleChannel*) ch, v, false);
		Fl::lock();
		editor->volume->value(v);
		Fl::unlock();
	}

	if (!gui) {
		Fl::lock();
		ch->guiChannel->vol->value(v);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void glue_setPitch(gdSampleEditor *win, SampleChannel *ch, float val, bool numeric)
{
	if (numeric) {
		if (val <= 0.0f)
			val = 0.1000f;
		if (val > 4.0f)
			val = 4.0000f;
		if (win)
			win->pitch->value(val);
	}

	ch->setPitch(val);

	if (win) {
		char buf[16];
		sprintf(buf, "%.4f", val);
		Fl::lock();
		win->pitchNum->value(buf);
		win->pitchNum->redraw();
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void glue_setPanning(gdSampleEditor *win, SampleChannel *ch, float val)
{
	if (val < 1.0f) {
		ch->panLeft = 1.0f;
		ch->panRight= 0.0f + val;

		char buf[8];
		sprintf(buf, "%d L", std::abs((int)((ch->panRight * 100.0f) - 100)));
		win->panNum->value(buf);
	}
	else if (val == 1.0f) {
		ch->panLeft = 1.0f;
		ch->panRight= 1.0f;
	  win->panNum->value("C");
	}
	else {
		ch->panLeft = 2.0f - val;
		ch->panRight= 1.0f;

		char buf[8];
		sprintf(buf, "%d R", std::abs((int)((ch->panLeft * 100.0f) - 100)));
		win->panNum->value(buf);
	}
	win->panNum->redraw();
}


/* -------------------------------------------------------------------------- */


void glue_setMute(Channel *ch, bool gui)
{
	if (recorder::active && recorder::canRec(ch, clock::isRunning(), mixer::recording)) {
		if (!ch->mute) {
			recorder::startOverdub(ch->index, G_ACTION_MUTES, clock::getCurrentFrame(),
        kernelAudio::getRealBufSize());
      ch->readActions = false;   // don't read actions while overdubbing
    }
		else
		 recorder::stopOverdub(clock::getCurrentFrame(), clock::getTotalFrames(),
      &mixer::mutex_recs);
	}

	ch->mute ? ch->unsetMute(false) : ch->setMute(false);

	if (!gui) {
		Fl::lock();
		ch->guiChannel->mute->value(ch->mute);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void glue_setSoloOn(Channel *ch, bool gui)
{
	/* if there's no solo session, store mute configuration of all chans
	 * and start the session */

	if (!__soloSession__) {
		for (unsigned i=0; i<mixer::channels.size(); i++) {
			Channel *och = mixer::channels.at(i);
			och->mute_s  = och->mute;
		}
		__soloSession__ = true;
	}

	ch->solo = !ch->solo;
	ch->sendMidiLsolo();

	/* mute all other channels and unmute this (if muted) */

	for (unsigned i=0; i<mixer::channels.size(); i++) {
		Channel *och = mixer::channels.at(i);
		if (!och->solo && !och->mute) {
			och->setMute(false);
			Fl::lock();
			och->guiChannel->mute->value(true);
			Fl::unlock();
		}
	}

	if (ch->mute) {
		ch->unsetMute(false);
		Fl::lock();
		ch->guiChannel->mute->value(false);
		Fl::unlock();
	}

	if (!gui) {
		Fl::lock();
		ch->guiChannel->solo->value(1);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void glue_setSoloOff(Channel *ch, bool gui)
{
	/* if this is uniqueSolo, stop solo session and restore mute status,
	 * else mute this */

	if (mh::uniqueSolo(ch)) {
		__soloSession__ = false;
		for (unsigned i=0; i<mixer::channels.size(); i++) {
			Channel *och = mixer::channels.at(i);
			if (och->mute_s) {
				och->setMute(false);
				Fl::lock();
				och->guiChannel->mute->value(true);
				Fl::unlock();
			}
			else {
				och->unsetMute(false);
				Fl::lock();
				och->guiChannel->mute->value(false);
				Fl::unlock();
			}
			och->mute_s = false;
		}
	}
	else {
		ch->setMute(false);
		Fl::lock();
		ch->guiChannel->mute->value(true);
		Fl::unlock();
	}

	ch->solo = !ch->solo;
	ch->sendMidiLsolo();

	if (!gui) {
		Fl::lock();
		ch->guiChannel->solo->value(0);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void glue_setBeginEndChannel(gdSampleEditor *win, SampleChannel *ch, int b, int e,
	bool recalc, bool check)
{
	if (check) {
		if (e > ch->wave->size)
			e = ch->wave->size;
		if (b < 0)
			b = 0;
		if (b > ch->wave->size)
			b = ch->wave->size-2;
		if (b >= ch->end)
			b = ch->begin;
		if (e <= ch->begin)
			e = ch->end;
	}

	/* continue only if new values != old values */

	if (b == ch->begin && e == ch->end)
		return;

	/* print mono values */

	char tmp[16];
	sprintf(tmp, "%d", b/2);
	win->chanStart->value(tmp);

	tmp[0] = '\0';
	sprintf(tmp, "%d", e/2);
	win->chanEnd->value(tmp);

	ch->setBegin(b);
	ch->setEnd(e);

	/* Recalc is not needed when the user drags the bars directly over the
	waveform */

	if (recalc) {
		win->waveTools->waveform->recalcPoints();
		win->waveTools->waveform->redraw();
	}
}


/* -------------------------------------------------------------------------- */


void glue_setBoost(gdSampleEditor *win, SampleChannel *ch, float val, bool numeric)
{
	if (numeric) {
		if (val > 20.0f)
			val = 20.0f;
		else if (val < 0.0f)
			val = 0.0f;

	  float linear = std::pow(10, (val / 20)); // linear = 10^(dB/20)

		ch->boost = linear;

		char buf[10];
		sprintf(buf, "%.2f", val);
		win->boostNum->value(buf);
		win->boostNum->redraw();

		win->boost->value(linear);
		win->boost->redraw();       /// inutile
	}
	else {
		ch->boost = val;
		char buf[10];
		sprintf(buf, "%.2f", 20 * std::log10(val));
		win->boostNum->value(buf);
		win->boostNum->redraw();
	}
}


/* -------------------------------------------------------------------------- */


void glue_setVolEditor(gdSampleEditor *win, SampleChannel *ch, float val, bool numeric)
{
	if (numeric) {
		if (val > 0.0f)
			val = 0.0f;
		else if (val < -60.0f)
			val = -INFINITY;

	  float linear = pow(10, (val / 20)); // linear = 10^(dB/20)

		ch->volume = linear;

		win->volume->value(linear);
		win->volume->redraw();

		char buf[10];
		if (val > -INFINITY)
			sprintf(buf, "%.2f", val);
		else
			sprintf(buf, "-inf");
		win->volumeNum->value(buf);
		win->volumeNum->redraw();

		ch->guiChannel->vol->value(linear);
		ch->guiChannel->vol->redraw();
	}
	else {
		ch->volume = val;

		float dbVal = 20 * log10(val);
		char buf[10];
		if (dbVal > -INFINITY)
			sprintf(buf, "%.2f", dbVal);
		else
			sprintf(buf, "-inf");

		win->volumeNum->value(buf);
		win->volumeNum->redraw();

		ch->guiChannel->vol->value(val);
		ch->guiChannel->vol->redraw();
	}
}


/* -------------------------------------------------------------------------- */


void glue_startStopReadingRecs(SampleChannel *ch, bool gui)
{
	/* When you call glue_startReadingRecs with conf::treatRecsAsLoops, the
	member value ch->readActions actually is not set to true immediately, because
	the channel is in wait mode (REC_WAITING). ch->readActions will become true on
	the next first beat. So a 'stop rec' command should occur also when
	ch->readActions is false but the channel is in wait mode; this check will
	handle the case of when you press 'R', the channel goes into REC_WAITING and
	then you press 'R' again to undo the status. */

	if (ch->readActions || (!ch->readActions && ch->recStatus == REC_WAITING))
		glue_stopReadingRecs(ch, gui);
	else
		glue_startReadingRecs(ch, gui);
}


/* -------------------------------------------------------------------------- */


void glue_startReadingRecs(SampleChannel *ch, bool gui)
{
	if (conf::treatRecsAsLoops)
		ch->recStatus = REC_WAITING;
	else
		ch->setReadActions(true, conf::recsStopOnChanHalt);
	if (!gui) {
		Fl::lock();
		((geSampleChannel*)ch->guiChannel)->readActions->value(1);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void glue_stopReadingRecs(SampleChannel *ch, bool gui)
{
	/* First of all, if the mixer is not running just stop and disable everything.
	Then if "treatRecsAsLoop" wait until the sequencer reaches beat 0, so put the
	channel in REC_ENDING status. */

	if (!clock::isRunning()) {
		ch->recStatus = REC_STOPPED;
		ch->readActions = false;
	}
	else
	if (conf::treatRecsAsLoops)
		ch->recStatus = REC_ENDING;
	else
		ch->setReadActions(false, conf::recsStopOnChanHalt);

	if (!gui) {
		Fl::lock();
		((geSampleChannel*)ch->guiChannel)->readActions->value(0);
		Fl::unlock();
	}
}
