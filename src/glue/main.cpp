/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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
#include <cassert>
#include <FL/Fl.H>
#include "gui/dialogs/warnings.h"
#include "gui/elems/mainWindow/mainIO.h"
#include "gui/elems/mainWindow/mainTimer.h"
#include "gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/dialogs/mainWindow.h"
#include "utils/gui.h"
#include "utils/string.h"
#include "utils/log.h"
#include "core/model/model.h"
#include "core/channels/midiChannel.h"
#include "core/mixerHandler.h"
#include "core/mixer.h"
#include "core/clock.h"
#include "core/init.h"
#include "core/kernelMidi.h"
#include "core/kernelAudio.h"
#include "core/recorder.h"
#include "core/recorderHandler.h"
#include "core/recManager.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/pluginManager.h"
#include "core/pluginHost.h"
#include "main.h"


extern giada::v::gdMainWindow *G_MainWin;


namespace giada {
namespace c {
namespace main
{
namespace
{
void setBpm_(float f, std::string s)
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

	float vPre = m::clock::getBpm();
	m::clock::setBpm(f);
	m::recorderHandler::updateBpm(vPre, f, m::clock::getQuanto());
	m::mixer::allocVirtualInput(m::clock::getFramesInLoop());

	/* This function might get called by Jack callback BEFORE the UI is up
	and running, that is when G_MainWin == nullptr. */
	
	if (G_MainWin != nullptr) {
		u::gui::refreshActionEditor();
		G_MainWin->mainTimer->setBpm(s.c_str());
	}

	gu_log("[glue::setBpm_] Bpm changed to %s (real=%f)\n", s.c_str(), m::clock::getBpm());
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void setBpm(const char* v1, const char* v2)
{
	/* Never change this stuff while recording audio. */

	if (m::recManager::isRecordingInput())
		return;

	/* A value such as atof("120.1") will never be 120.1 but 120.0999999, because 
	of the rounding error. So we pass the real "wrong" value to mixer and we show 
	the nice looking (but fake) one to the GUI. 
	On Linux, let Jack handle the bpm change if its on. */

	float       f = std::atof(v1) + (std::atof(v2)/10);
	std::string s = std::string(v1) + "." + std::string(v2);

#ifdef G_OS_LINUX
	if (m::kernelAudio::getAPI() == G_SYS_API_JACK)
		m::kernelAudio::jackSetBpm(f);
	else
#endif
	setBpm_(f, s);
}


/* -------------------------------------------------------------------------- */


void setBpm(float f)
{
	/* Never change this stuff while recording audio. */

	if (m::recManager::isRecordingInput())
		return;

	float intpart;
	float fracpart = std::round(std::modf(f, &intpart) * 10);
	std::string s = std::to_string((int) intpart) + "." + std::to_string((int)fracpart);

	setBpm_(f, s);
}


/* -------------------------------------------------------------------------- */


void setBeats(int beats, int bars)
{
	/* Never change this stuff while recording audio */

	if (m::recManager::isRecordingInput())
		return;

	m::clock::setBeats(beats, bars);
	m::mixer::allocVirtualInput(m::clock::getFramesInLoop());

	G_MainWin->mainTimer->setMeter(m::clock::getBeats(), m::clock::getBars());
	u::gui::refreshActionEditor();  // in case the action editor is open
}


/* -------------------------------------------------------------------------- */


void quantize(int val)
{
	m::clock::setQuantize(val);
}


/* -------------------------------------------------------------------------- */


void setOutVol(float v, bool gui)
{
	m::mh::setOutVol(v);
	
	if (!gui) {
		Fl::lock();
		G_MainWin->mainIO->setOutVol(v);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void setInVol(float v, bool gui)
{
	m::mh::setInVol(v);

	if (!gui) {
		Fl::lock();
		G_MainWin->mainIO->setInVol(v);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void clearAllSamples()
{
	if (!v::gdConfirmWin("Warning", "Clear all samples: are you sure?"))
		return;
	G_MainWin->delSubWindow(WID_SAMPLE_EDITOR);
	m::clock::setStatus(ClockStatus::STOPPED);
	m::mh::freeAllChannels();
	m::recorder::clearAll();
	return;
}


/* -------------------------------------------------------------------------- */


void clearAllActions()
{
	if (!v::gdConfirmWin("Warning", "Clear all actions: are you sure?"))
		return;
	G_MainWin->delSubWindow(WID_ACTION_EDITOR);
	//for (std::unique_ptr<m::Channel>& ch : m::model::getLayout()->channels)
	//	ch->hasActions = false;
	m::recorder::clearAll();
}


/* -------------------------------------------------------------------------- */


void resetToInitState(bool createColumns)
{
	if (!v::gdConfirmWin("Warning", "Reset to init state: are you sure?"))
		return;
	m::init::reset();	
	m::mixer::enable();
}


/* -------------------------------------------------------------------------- */


void beatsMultiply()
{
	setBeats(m::clock::getBeats() * 2, m::clock::getBars());
}

void beatsDivide()
{
	setBeats(m::clock::getBeats() / 2, m::clock::getBars());
}


/* -------------------------------------------------------------------------- */


void toggleInputRec()
{
	if (!m::recManager::toggleInputRec(static_cast<RecTriggerMode>(m::conf::recTriggerMode)))
		v::gdAlert("No channels armed/available for audio recording.");
}


/* -------------------------------------------------------------------------- */


void toggleActionRec()
{
	m::recManager::isRecordingAction() ? stopActionRec() : startActionRec();
}


void startActionRec()
{
	m::recManager::startActionRec(static_cast<RecTriggerMode>(m::conf::recTriggerMode));
}


void stopActionRec()
{
	m::recManager::stopActionRec();
	u::gui::refreshActionEditor();  // If Action Editor window is open
}

}}} // giada::c::main::
