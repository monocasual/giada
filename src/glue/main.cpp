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


#include <cmath>
#include "../gui/elems/ge_waveform.h"
#include "../gui/elems/ge_mixed.h"
#include "../gui/elems/ge_waveTools.h"
#include "../gui/elems/mainWindow/mainTransport.h"
#include "../gui/elems/mainWindow/mainIO.h"
#include "../gui/elems/mainWindow/mainTimer.h"
#include "../gui/elems/mainWindow/keyboard/channel.h"
#include "../gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "../gui/elems/mainWindow/keyboard/keyboard.h"
#include "../gui/dialogs/gd_mainWindow.h"
#include "../gui/dialogs/gd_editor.h"
#include "../gui/dialogs/gd_warnings.h"
#include "../utils/gui.h"
#include "../utils/fs.h"
#include "../utils/log.h"
#include "../core/mixerHandler.h"
#include "../core/mixer.h"
#include "../core/recorder.h"
#include "../core/wave.h"
#include "../core/pluginHost.h"
#include "../core/channel.h"
#include "../core/sampleChannel.h"
#include "../core/midiChannel.h"
#include "../core/kernelMidi.h"
#include "../core/patch_DEPR_.h"
#include "../core/conf.h"
#include "main.h"


extern gdMainWindow *G_MainWin;
extern Mixer	   		 G_Mixer;
extern Recorder			 G_Recorder;
extern KernelAudio   G_KernelAudio;
extern KernelMidi    G_KernelMidi;
extern Patch_DEPR_   G_Patch_DEPR_;
extern Conf	 	   		 G_Conf;
extern bool 		 		 G_audio_status;
#ifdef WITH_VST
extern PluginHost    G_PluginHost;
#endif


void glue_setBpm(const char *v1, const char *v2)
{
  /* Never change this stuff while recording audio */

  if (G_Mixer.recording)
    return;

	char  buf[6];
	float value = atof(v1) + (atof(v2)/10);
	if (value < 20.0f)	{
		value = 20.0f;
		sprintf(buf, "20.0");
	}
	else
		sprintf(buf, "%s.%s", v1, !strcmp(v2, "") ? "0" : v2);

	/* a value such as atof("120.1") will never be 120.1 but 120.0999999,
	 * because of the rounding error. So we pass the real "wrong" value to
	 * G_Mixer and we show the nice looking (but fake) one to the GUI. */

	float old_bpm = G_Mixer.bpm;
	G_Mixer.bpm = value;
	G_Mixer.updateFrameBars();

	/* inform recorder and actionEditor of the change */

	G_Recorder.updateBpm(old_bpm, value, G_Mixer.quanto);
	gu_refreshActionEditor();

	G_MainWin->mainTimer->setBpm(buf);
	gu_log("[glue] Bpm changed to %s (real=%f)\n", buf, G_Mixer.bpm);
}


/* -------------------------------------------------------------------------- */


void glue_setBeats(int beats, int bars, bool expand)
{
  /* Never change this stuff while recording audio */

  if (G_Mixer.recording)
    return;

	/* Temp vars to store old data (they are necessary) */

	int      oldvalue = G_Mixer.beats;
	unsigned oldfpb		= G_Mixer.totalFrames;

	if (beats > MAX_BEATS)
		G_Mixer.beats = MAX_BEATS;
	else if (beats < 1)
		G_Mixer.beats = 1;
	else
		G_Mixer.beats = beats;

	/* update bars - bars cannot be greate than beats and must be a sub
	 * multiple of beats. If not, approximation to the nearest (and greater)
	 * value available. */

	if (bars > G_Mixer.beats)
		G_Mixer.bars = G_Mixer.beats;
	else if (bars <= 0)
		G_Mixer.bars = 1;
	else if (beats % bars != 0) {
		G_Mixer.bars = bars + (beats % bars);
		if (beats % G_Mixer.bars != 0) // it could be an odd value, let's check it (and avoid it)
			G_Mixer.bars = G_Mixer.bars - (beats % G_Mixer.bars);
	}
	else
		G_Mixer.bars = bars;

	G_Mixer.updateFrameBars();

	/* update recorded actions */

	if (expand) {
		if (G_Mixer.beats > oldvalue)
			G_Recorder.expand(oldfpb, G_Mixer.totalFrames);
		//else if (G_Mixer.beats < oldvalue)
		//	G_Recorder.shrink(G_Mixer.totalFrames);
	}

	G_MainWin->mainTimer->setMeter(G_Mixer.beats, G_Mixer.bars);
	gu_refreshActionEditor();  // in case the action editor is open
}


/* -------------------------------------------------------------------------- */


void glue_startStopSeq(bool gui)
{
	G_Mixer.running ? glue_stopSeq(gui) : glue_startSeq(gui);
}


/* -------------------------------------------------------------------------- */


void glue_startSeq(bool gui)
{
	G_Mixer.running = true;

	if (gui) {
#ifdef __linux__
		G_KernelAudio.jackStart();
#endif
	}

	if (G_Conf.midiSync == MIDI_SYNC_CLOCK_M) {
		G_KernelMidi.send(MIDI_START, -1, -1);
		G_KernelMidi.send(MIDI_POSITION_PTR, 0, 0);
	}

	if (!gui) {
    Fl::lock();
    G_MainWin->mainTransport->updatePlay(1);
    Fl::unlock();
  }
}


/* -------------------------------------------------------------------------- */


void glue_stopSeq(bool gui)
{
	mh_stopSequencer();

	if (G_Conf.midiSync == MIDI_SYNC_CLOCK_M)
		G_KernelMidi.send(MIDI_STOP, -1, -1);

#ifdef __linux__
	if (gui)
		G_KernelAudio.jackStop();
#endif

	/* what to do if we stop the sequencer and some action recs are active?
	 * Deactivate the button and delete any 'rec on' status */

	if (G_Recorder.active) {
		G_Recorder.active = false;
    Fl::lock();
	  G_MainWin->mainTransport->updateRecAction(0);
	  Fl::unlock();
	}

	/* if input recs are active (who knows why) we must deactivate them.
	 * One might stop the sequencer while an input rec is running. */

	if (G_Mixer.recording) {
		mh_stopInputRec();
    Fl::lock();
	  G_MainWin->mainTransport->updateRecInput(0);
	  Fl::unlock();
	}

	if (!gui) {
    Fl::lock();
	  G_MainWin->mainTransport->updatePlay(0);
	  Fl::unlock();
  }
}


/* -------------------------------------------------------------------------- */


void glue_rewindSeq()
{
	mh_rewindSequencer();
	if (G_Conf.midiSync == MIDI_SYNC_CLOCK_M)
		G_KernelMidi.send(MIDI_POSITION_PTR, 0, 0);
}


/* -------------------------------------------------------------------------- */


void glue_startStopReadingRecs(SampleChannel *ch, bool gui)
{
	/* When you call glue_startReadingRecs with G_Conf.treatRecsAsLoops, the
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
	if (G_Conf.treatRecsAsLoops)
		ch->recStatus = REC_WAITING;
	else
		ch->setReadActions(true, G_Conf.recsStopOnChanHalt);
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

	if (!G_Mixer.running) {
		ch->recStatus = REC_STOPPED;
		ch->readActions = false;
	}
	else
	if (G_Conf.treatRecsAsLoops)
		ch->recStatus = REC_ENDING;
	else
		ch->setReadActions(false, G_Conf.recsStopOnChanHalt);

	if (!gui) {
		Fl::lock();
		((geSampleChannel*)ch->guiChannel)->readActions->value(0);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void glue_quantize(int val)
{
	G_Mixer.quantize = val;
	G_Mixer.updateQuanto();
}


/* -------------------------------------------------------------------------- */


void glue_setOutVol(float v, bool gui)
{
	G_Mixer.outVol = v;
	if (!gui) {
		Fl::lock();
		G_MainWin->mainIO->setOutVol(v);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void glue_setInVol(float v, bool gui)
{
	G_Mixer.inVol = v;
	if (!gui) {
		Fl::lock();
		G_MainWin->mainIO->setInVol(v);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void glue_clearAllSamples()
{
	G_Mixer.running = false;
	for (unsigned i=0; i<G_Mixer.channels.size(); i++) {
		G_Mixer.channels.at(i)->empty();
		G_Mixer.channels.at(i)->guiChannel->reset();
	}
	G_Recorder.init();
	return;
}


/* -------------------------------------------------------------------------- */


void glue_clearAllRecs()
{
	G_Recorder.init();
	gu_updateControls();
}


/* -------------------------------------------------------------------------- */


void glue_resetToInitState(bool resetGui, bool createColumns)
{
	G_Patch_DEPR_.setDefault();
	G_Mixer.close();
	G_Mixer.init();
	G_Recorder.init();
#ifdef WITH_VST
	G_PluginHost.freeAllStacks(&G_Mixer.channels, &G_Mixer.mutex_plugins);
#endif

	G_MainWin->keyboard->clear();
	if (createColumns)
		G_MainWin->keyboard->init();

  gu_updateMainWinLabel(G_DEFAULT_PATCH_NAME);

	if (resetGui)
		gu_updateControls();
}


/* -------------------------------------------------------------------------- */


void glue_startStopMetronome(bool gui)
{
	G_Mixer.metronome = !G_Mixer.metronome;
	if (!gui) {
		Fl::lock();
		G_MainWin->mainTransport->updateMetronome(G_Mixer.metronome);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


/* never expand or shrink recordings (last param of setBeats = false):
 * this is live manipulation */

void glue_beatsMultiply()
{
	glue_setBeats(G_Mixer.beats*2, G_Mixer.bars, false);
}

void glue_beatsDivide()
{
	glue_setBeats(G_Mixer.beats/2, G_Mixer.bars, false);
}
