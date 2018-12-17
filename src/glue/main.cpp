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


#include <cmath>
#include <FL/Fl.H>
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
#include "../core/kernelAudio.h"
#include "../core/recorder/recorder.h"
#include "../core/recorderHandler.h"
#include "../core/conf.h"
#include "../core/const.h"
#ifdef WITH_VST
#include "../core/pluginHost.h"
#endif
#include "main.h"


extern gdMainWindow *G_MainWin;


using std::string;
using namespace giada::m;


namespace
{
void setBpm_(float f, string s)
{
	if (f < G_MIN_BPM) {
		f = G_MIN_BPM;
		s = G_MIN_BPM_STR;
	}
	else
	if (f > G_MAX_BPM) {
		f = G_MAX_BPM;
		s = G_MAX_BPM_STR;		
	}

	float vPre = clock::getBpm();
	clock::setBpm(f);
	recorderHandler::updateBpm(vPre, f, clock::getQuanto());
	mixer::allocVirtualInput(clock::getFramesInLoop());

	gu_refreshActionEditor();
	G_MainWin->mainTimer->setBpm(s.c_str());

	gu_log("[glue::setBpm_] Bpm changed to %s (real=%f)\n", s.c_str(), clock::getBpm());
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void glue_setBpm(const char* v1, const char* v2)
{
	/* Never change this stuff while recording audio */

	if (mixer::recording)
		return;

	/* A value such as atof("120.1") will never be 120.1 but 120.0999999, because 
	of the rounding error. So we pass the real "wrong" value to mixer and we show 
	the nice looking (but fake) one to the GUI. 
	On Linux, let Jack handle the bpm change if its on. */

	float  f = atof(v1) + (atof(v2)/10);
	string s = string(v1) + "." + string(v2);

#ifdef G_OS_LINUX
	if (kernelAudio::getAPI() == G_SYS_API_JACK)
		kernelAudio::jackSetBpm(f);
	else
#endif
	setBpm_(f, s);
}


/* -------------------------------------------------------------------------- */


void glue_setBpm(float f)
{
	/* Never change this stuff while recording audio */

	if (mixer::recording)
		return;

	float intpart;
	float fracpart = std::round(std::modf(f, &intpart) * 10);
	string s = std::to_string((int) intpart) + "." + std::to_string((int)fracpart);

	setBpm_(f, s);
}


/* -------------------------------------------------------------------------- */


void glue_setBeats(int beats, int bars)
{
	/* Never change this stuff while recording audio */

	if (mixer::recording)
		return;

	clock::setBeats(beats);
	clock::setBars(bars);
	clock::updateFrameBars();
	mixer::allocVirtualInput(clock::getFramesInLoop());

	G_MainWin->mainTimer->setMeter(clock::getBeats(), clock::getBars());
	gu_refreshActionEditor();  // in case the action editor is open
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
	for (Channel* ch : mixer::channels) {
		ch->empty();
		ch->guiChannel->reset();
	}
	recorder::clearAll();
	return;
}


/* -------------------------------------------------------------------------- */


void glue_clearAllActions()
{
	recorder::clearAll();
	for (Channel* ch : mixer::channels)
		ch->hasActions = false;
	gu_updateControls();
}


/* -------------------------------------------------------------------------- */


void glue_resetToInitState(bool resetGui, bool createColumns)
{
	gu_closeAllSubwindows();
	mixer::close();
	clock::init(conf::samplerate, conf::midiTCfps);
	mixer::init(clock::getFramesInLoop(), kernelAudio::getRealBufSize());
	recorder::init(&mixer::mutex);
#ifdef WITH_VST
	pluginHost::freeAllStacks(&mixer::channels, &mixer::mutex);
#endif

	G_MainWin->keyboard->clear();
	if (createColumns)
		G_MainWin->keyboard->init();

	gu_updateMainWinLabel(G_DEFAULT_PATCH_NAME);

	if (resetGui)
		gu_updateControls();
}


/* -------------------------------------------------------------------------- */


void glue_beatsMultiply()
{
	glue_setBeats(clock::getBeats() * 2, clock::getBars());
}

void glue_beatsDivide()
{
	glue_setBeats(clock::getBeats() / 2, clock::getBars());
}
