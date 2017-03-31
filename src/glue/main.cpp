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
#include "../gui/elems/mainWindow/mainIO.h"
#include "../gui/elems/mainWindow/mainTimer.h"
#include "../gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "../gui/elems/mainWindow/keyboard/keyboard.h"
#include "../gui/dialogs/gd_mainWindow.h"
#include "../utils/gui.h"
#include "../utils/string.h"
#include "../utils/log.h"
#include "../core/mixerHandler.h"
#include "../core/mixer.h"
#include "../core/midiChannel.h"
#include "../core/clock.h"
#include "../core/kernelMidi.h"
#include "../core/patch_DEPR_.h"
#include "../core/conf.h"
#ifdef WITH_VST
#include "../core/pluginHost.h"
#endif
#include "main.h"


extern gdMainWindow *G_MainWin;
extern Recorder			 G_Recorder;
extern Patch_DEPR_   G_Patch_DEPR_;
#ifdef WITH_VST
extern PluginHost    G_PluginHost;
#endif


using namespace giada;


void glue_setBpm(const char *v1, const char *v2)
{
  /* Never change this stuff while recording audio */

  if (mixer::recording)
    return;

	char  bpmS[6];
	float bpmF = atof(v1) + (atof(v2)/10);
	if (bpmF < 20.0f) {
		bpmF = 20.0f;
		sprintf(bpmS, "20.0");
	}
	else
		sprintf(bpmS, "%s.%s", v1, !strcmp(v2, "") ? "0" : v2);

	/* a value such as atof("120.1") will never be 120.1 but 120.0999999,
	 * because of the rounding error. So we pass the real "wrong" value to
	 * G_Mixer and we show the nice looking (but fake) one to the GUI. */

	float oldBpmF = clock::getBpm();
	clock::setBpm(bpmF);
  G_Recorder.updateBpm(oldBpmF, bpmF, clock::getQuanto());

#ifdef __linux__
  kernelAudio::jackSetBpm(clock::getBpm());
#endif

  gu_refreshActionEditor();
  G_MainWin->mainTimer->setBpm(bpmS);

	gu_log("[glue] Bpm changed to %s (real=%f)\n", bpmS, clock::getBpm());
}


/* -------------------------------------------------------------------------- */


void glue_setBpm(float v)
{
  if (v < G_MIN_BPM || v > G_MAX_BPM)
    v = G_DEFAULT_BPM;
  double fIpart;
  double fPpart = modf(v, &fIpart);
  int iIpart = fIpart;
  int iPpart = ceilf(fPpart);
  glue_setBpm(gu_itoa(iIpart).c_str(), gu_itoa(iPpart).c_str());
}


/* -------------------------------------------------------------------------- */


void glue_setBeats(int beats, int bars, bool expand)
{
  /* Never change this stuff while recording audio */

  if (mixer::recording)
    return;

	/* Temp vars to store old data (they are necessary) */

	int      oldvalue = clock::getBeats();
	unsigned oldfpb		= clock::getTotalFrames();

	if (beats > G_MAX_BEATS)
		clock::setBeats(G_MAX_BEATS);
	else if (beats < 1)
		clock::setBeats(1);
	else
		clock::setBeats(beats);

	/* update bars - bars cannot be greate than beats and must be a sub
	 * multiple of beats. If not, approximation to the nearest (and greater)
	 * value available. */
  /* TODO - move this to Clock*/

	if (bars > clock::getBeats())
		clock::setBars(clock::getBeats());
	else if (bars <= 0)
		clock::setBars(1);
	else if (beats % bars != 0) {
		clock::setBars(bars + (beats % bars));
		if (beats % clock::getBars() != 0) // it could be an odd value, let's check it (and avoid it)
			clock::setBars(clock::getBars() - (beats % clock::getBars()));
	}
	else
		clock::setBars(bars);

	clock::updateFrameBars();

	/* update recorded actions */

	if (expand) {
		if (clock::getBeats() > oldvalue)
			G_Recorder.expand(oldfpb, clock::getTotalFrames());
		//else if (mixer::beats < oldvalue)
		//	G_Recorder.shrink(mixer::totalFrames);
	}

	G_MainWin->mainTimer->setMeter(clock::getBeats(), clock::getBars());
	gu_refreshActionEditor();  // in case the action editor is open
}


/* -------------------------------------------------------------------------- */


void glue_rewindSeq(bool gui, bool notifyJack)
{
	mh::rewindSequencer();

  /* FIXME - potential desync when Quantizer is enabled from this point on.
  Mixer would wait, while the following calls would be made regardless of its
  state. */

#ifdef __linux__
  if (notifyJack)
	  kernelAudio::jackSetPosition(0);
#endif

	if (conf::midiSync == MIDI_SYNC_CLOCK_M)
		kernelMidi::send(MIDI_POSITION_PTR, 0, 0);
}


/* -------------------------------------------------------------------------- */


void glue_quantize(int val)
{
	clock::setQuantize(val);
}


/* -------------------------------------------------------------------------- */


void glue_setOutVol(float v, bool gui)
{
	mixer::outVol = v;
	if (!gui) {
		Fl::lock();
		G_MainWin->mainIO->setOutVol(v);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void glue_setInVol(float v, bool gui)
{
	mixer::inVol = v;
	if (!gui) {
		Fl::lock();
		G_MainWin->mainIO->setInVol(v);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void glue_clearAllSamples()
{
	clock::stop();
	for (unsigned i=0; i<mixer::channels.size(); i++) {
		mixer::channels.at(i)->empty();
		mixer::channels.at(i)->guiChannel->reset();
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
	mixer::close();
	mixer::init();
	G_Recorder.init();
#ifdef WITH_VST
	G_PluginHost.freeAllStacks(&mixer::channels, &mixer::mutex_plugins);
#endif

	G_MainWin->keyboard->clear();
	if (createColumns)
		G_MainWin->keyboard->init();

  gu_updateMainWinLabel(G_DEFAULT_PATCH_NAME);

	if (resetGui)
		gu_updateControls();
}


/* -------------------------------------------------------------------------- */


/* never expand or shrink recordings (last param of setBeats = false):
 * this is live manipulation */

void glue_beatsMultiply()
{
	glue_setBeats(clock::getBeats() * 2, clock::getBars(), false);
}

void glue_beatsDivide()
{
	glue_setBeats(clock::getBeats() / 2, clock::getBars(), false);
}
