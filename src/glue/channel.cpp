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
#include <FL/Fl.H>
#include "../gui/dialogs/gd_mainWindow.h"
#include "../gui/dialogs/sampleEditor.h"
#include "../gui/dialogs/gd_warnings.h"
#include "../gui/elems/basics/input.h"
#include "../gui/elems/basics/dial.h"
#include "../gui/elems/sampleEditor/waveTools.h"
#include "../gui/elems/sampleEditor/volumeTool.h"
#include "../gui/elems/sampleEditor/boostTool.h"
#include "../gui/elems/sampleEditor/panTool.h"
#include "../gui/elems/sampleEditor/pitchTool.h"
#include "../gui/elems/sampleEditor/rangeTool.h"
#include "../gui/elems/sampleEditor/waveform.h"
#include "../gui/elems/mainWindow/keyboard/keyboard.h"
#include "../gui/elems/mainWindow/keyboard/channel.h"
#include "../gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "../gui/elems/mainWindow/keyboard/channelButton.h"
#include "../utils/gui.h"
#include "../utils/fs.h"
#include "../utils/log.h"
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
#include "../core/waveManager.h"
#include "main.h"
#include "channel.h"


extern gdMainWindow* G_MainWin;


using std::string;
using namespace giada::m;


static bool __soloSession__ = false;


int glue_loadChannel(SampleChannel* ch, const string& fname)
{
	/* Always stop a channel before loading a new sample in it. This will prevent
	issues if tracker is outside the boundaries of the new sample -> segfault. */

	if (ch->status & (STATUS_PLAY | STATUS_ENDING))
		ch->hardStop(0);

	/* Save the patch and take the last browser's dir in order to re-use it the 
	next time. */

	conf::samplePath = gu_dirname(fname);

	Wave* wave = nullptr;
	int result = waveManager::create(fname, &wave); 
	if (result != G_RES_OK)
		return result;

	if (wave->getRate() != conf::samplerate) {
		gu_log("[glue_loadChannel] input rate (%d) != system rate (%d), conversion needed\n",
			wave->getRate(), conf::samplerate);
		result = waveManager::resample(wave, conf::rsmpQuality, conf::samplerate); 
		if (result != G_RES_OK) {
			delete wave;
			return result;
		}
	}

	ch->pushWave(wave);

	G_MainWin->keyboard->updateChannel(ch->guiChannel);

	return result;
}


/* -------------------------------------------------------------------------- */


Channel* glue_addChannel(int column, int type, int size)
{
	Channel* ch    = mh::addChannel(type);
	geChannel* gch = G_MainWin->keyboard->addChannel(column, ch, size);
	ch->guiChannel = gch;
	return ch;
}


/* -------------------------------------------------------------------------- */


void glue_deleteChannel(Channel* ch)
{
	if (!gdConfirmWin("Warning", "Delete channel: are you sure?"))
		return;
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


void glue_freeChannel(Channel* ch)
{
	if (ch->status == STATUS_PLAY) {
		if (!gdConfirmWin("Warning", "This action will stop the channel: are you sure?"))
			return;
	}
	else
	if (!gdConfirmWin("Warning", "Free channel: are you sure?"))
		return;

	G_MainWin->keyboard->freeChannel(ch->guiChannel);
	recorder::clearChan(ch->index);
	ch->hasActions = false;
	ch->empty();

	/* delete any related subwindow */
	/** TODO - use gu_closeAllSubwindows()   */
	G_MainWin->delSubWindow(WID_FILE_BROWSER);
	G_MainWin->delSubWindow(WID_ACTION_EDITOR);
	G_MainWin->delSubWindow(WID_SAMPLE_EDITOR);
	G_MainWin->delSubWindow(WID_FX_LIST);
}


/* -------------------------------------------------------------------------- */


void glue_toggleArm(Channel* ch, bool gui)
{
	ch->setArmed(!ch->isArmed());
	if (!gui)
		ch->guiChannel->arm->value(ch->isArmed());
}


/* -------------------------------------------------------------------------- */


void glue_toggleInputMonitor(Channel* ch)
{
	SampleChannel* sch = static_cast<SampleChannel*>(ch);
	sch->inputMonitor = !sch->inputMonitor;
}


/* -------------------------------------------------------------------------- */


int glue_cloneChannel(Channel* src)
{
	Channel* ch    = mh::addChannel(src->type);
	geChannel* gch = G_MainWin->keyboard->addChannel(src->guiChannel->getColumnIndex(), 
		ch, src->guiChannel->getSize());

	ch->guiChannel = gch;
	ch->copy(src, &mixer::mutex_plugins);

	G_MainWin->keyboard->updateChannel(ch->guiChannel);
	return true;
}


/* -------------------------------------------------------------------------- */


void glue_setVolume(Channel *ch, float v, bool gui, bool editor)
{
	ch->volume = v;

	/* Changing channel volume? Update wave editor (if it's shown). */

	if (!editor) {
		gdSampleEditor *gdEditor = (gdSampleEditor*) gu_getSubwindow(G_MainWin, WID_SAMPLE_EDITOR);
		if (gdEditor) {
			Fl::lock();
			gdEditor->volumeTool->refresh();
			Fl::unlock();
		}
	}

	if (!gui) {
		Fl::lock();
		ch->guiChannel->vol->value(v);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void glue_setPitch(SampleChannel* ch, float val)
{
	ch->setPitch(val);
	gdSampleEditor* gdEditor = static_cast<gdSampleEditor*>(gu_getSubwindow(G_MainWin, WID_SAMPLE_EDITOR));
	if (gdEditor) {
		Fl::lock();
		gdEditor->pitchTool->refresh();
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void glue_setPanning(SampleChannel* ch, float val)
{
	ch->setPan(val);
	gdSampleEditor* gdEditor = static_cast<gdSampleEditor*>(gu_getSubwindow(G_MainWin, WID_SAMPLE_EDITOR));
	if (gdEditor) {
		Fl::lock();
		gdEditor->panTool->refresh();
		Fl::unlock();
	}
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


void glue_setBoost(SampleChannel *ch, float val)
{
	ch->setBoost(val);
	gdSampleEditor *gdEditor = static_cast<gdSampleEditor*>(gu_getSubwindow(G_MainWin, WID_SAMPLE_EDITOR));
	if (gdEditor) {
		Fl::lock();
		gdEditor->boostTool->refresh();
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void glue_setName(Channel* ch, const string& name)
{
	ch->setName(name);
	ch->guiChannel->update();
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
