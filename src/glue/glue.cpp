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
#include "../gui/elems/channel.h"
#include "../gui/elems/sampleChannel.h"
#include "../gui/elems/ge_waveTools.h"
#include "../gui/elems/ge_keyboard.h"
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
#include "glue.h"


extern gdMainWindow *G_MainWin;
extern Mixer	   		 G_Mixer;
extern Recorder			 G_Recorder;
extern KernelMidi    G_KernelMidi;
extern Patch_DEPR_   G_Patch_DEPR_;
extern Conf	 	   		 G_Conf;
extern bool 		 		 G_audio_status;
#ifdef WITH_VST
extern PluginHost    G_PluginHost;
#endif


static bool __soloSession__ = false;


void glue_setBpm(const char *v1, const char *v2)
{
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

	G_MainWin->timing->setBpm(buf);
	gu_log("[glue] Bpm changed to %s (real=%f)\n", buf, G_Mixer.bpm);
}


/* -------------------------------------------------------------------------- */


void glue_setBeats(int beats, int bars, bool expand)
{
	/* temp vars to store old data (they are necessary) */

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

	G_MainWin->timing->setMeter(G_Mixer.beats, G_Mixer.bars);
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
		kernelAudio::jackStart();
#endif
	}

	if (G_Conf.midiSync == MIDI_SYNC_CLOCK_M) {
		G_KernelMidi.send(MIDI_START, -1, -1);
		G_KernelMidi.send(MIDI_POSITION_PTR, 0, 0);
	}

	if (gui) Fl::lock();
	G_MainWin->controller->updatePlay(1);
	if (gui) Fl::unlock();
}


/* -------------------------------------------------------------------------- */


void glue_stopSeq(bool gui)
{
	mh_stopSequencer();

	if (G_Conf.midiSync == MIDI_SYNC_CLOCK_M)
		G_KernelMidi.send(MIDI_STOP, -1, -1);

#ifdef __linux__
	if (gui)
		kernelAudio::jackStop();
#endif

	/* what to do if we stop the sequencer and some action recs are active?
	 * Deactivate the button and delete any 'rec on' status */

	if (G_Recorder.active) {
		G_Recorder.active = false;
		if (gui) Fl::lock();
		G_MainWin->controller->updateRecAction(0);
		if (gui) Fl::unlock();
	}

	/* if input recs are active (who knows why) we must deactivate them.
	 * One might stop the sequencer while an input rec is running. */

	if (G_Mixer.chanInput != NULL) {
		mh_stopInputRec();
		if (gui) Fl::lock();
		G_MainWin->controller->updateRecInput(0);
		if (gui) Fl::unlock();
	}

	if (gui) Fl::lock();
	G_MainWin->controller->updatePlay(0);
	if (gui) Fl::unlock();
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


void glue_setChanVol(Channel *ch, float v, bool gui)
{
	ch->volume = v;

	/* also update wave editor if it's shown */

	gdEditor *editor = (gdEditor*) gu_getSubwindow(G_MainWin, WID_SAMPLE_EDITOR);
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


void glue_setOutVol(float v, bool gui)
{
	G_Mixer.outVol = v;
	if (!gui) {
		Fl::lock();
		G_MainWin->inOut->setOutVol(v);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void glue_setInVol(float v, bool gui)
{
	G_Mixer.inVol = v;
	if (!gui) {
		Fl::lock();
		G_MainWin->inOut->setInVol(v);
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

	if (resetGui)
		gu_updateControls();
}


/* -------------------------------------------------------------------------- */


void glue_startStopMetronome(bool gui)
{
	G_Mixer.metronome = !G_Mixer.metronome;
	if (!gui) {
		Fl::lock();
		G_MainWin->controller->updateMetronome(G_Mixer.metronome);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void glue_setBeginEndChannel(gdEditor *win, SampleChannel *ch, int b, int e, bool recalc, bool check)
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

	/* recalc is not needed when the user drags the bars directly over the waveform */

	if (recalc) {
		win->waveTools->waveform->recalcPoints();	// importante, altrimenti non si vedono
		win->waveTools->waveform->redraw();
	}
}


/* -------------------------------------------------------------------------- */


void glue_setBoost(gdEditor *win, SampleChannel *ch, float val, bool numeric)
{
	if (numeric) {
		if (val > 20.0f)
			val = 20.0f;
		else if (val < 0.0f)
			val = 0.0f;

	  float linear = pow(10, (val / 20)); // linear = 10^(dB/20)

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
		sprintf(buf, "%.2f", 20*log10(val));
		win->boostNum->value(buf);
		win->boostNum->redraw();
	}
}


/* -------------------------------------------------------------------------- */


void glue_setVolEditor(class gdEditor *win, SampleChannel *ch, float val, bool numeric)
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


void glue_setMute(Channel *ch, bool gui)
{
	if (G_Recorder.active && G_Recorder.canRec(ch)) {
		if (!ch->mute)
			G_Recorder.startOverdub(ch->index, ACTION_MUTES, G_Mixer.actualFrame);
		else
		 G_Recorder.stopOverdub(G_Mixer.actualFrame);
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
		for (unsigned i=0; i<G_Mixer.channels.size(); i++) {
			Channel *och = G_Mixer.channels.at(i);
			och->mute_s  = och->mute;
		}
		__soloSession__ = true;
	}

	ch->solo = !ch->solo;
	ch->sendMidiLsolo();

	/* mute all other channels and unmute this (if muted) */

	for (unsigned i=0; i<G_Mixer.channels.size(); i++) {
		Channel *och = G_Mixer.channels.at(i);
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

	if (mh_uniqueSolo(ch)) {
		__soloSession__ = false;
		for (unsigned i=0; i<G_Mixer.channels.size(); i++) {
			Channel *och = G_Mixer.channels.at(i);
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


void glue_setPanning(class gdEditor *win, SampleChannel *ch, float val)
{
	if (val < 1.0f) {
		ch->panLeft = 1.0f;
		ch->panRight= 0.0f + val;

		char buf[8];
		sprintf(buf, "%d L", (int) std::abs((ch->panRight * 100.0f) - 100));
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
		sprintf(buf, "%d R", (int) std::abs((ch->panLeft * 100.0f) - 100));
		win->panNum->value(buf);
	}
	win->panNum->redraw();
}


/* -------------------------------------------------------------------------- */


void glue_setPitch(class gdEditor *win, SampleChannel *ch, float val, bool numeric)
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
