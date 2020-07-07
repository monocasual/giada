/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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
void setBpm_(float current, std::string s)
{
	if (current < G_MIN_BPM) {
		current = G_MIN_BPM;
		s = G_MIN_BPM_STR;
	}
	else
	if (current > G_MAX_BPM) {
		current = G_MAX_BPM;
		s = G_MAX_BPM_STR;		
	}

	float previous = m::clock::getBpm();
	m::clock::setBpm(current);
	m::recorderHandler::updateBpm(previous, current, m::clock::getQuantizerStep());
	m::mixer::allocRecBuffer(m::clock::getFramesInLoop());

	/* This function might get called by Jack callback BEFORE the UI is up
	and running, that is when G_MainWin == nullptr. */
	
	if (G_MainWin != nullptr) {
		u::gui::refreshActionEditor();
		G_MainWin->mainTimer->setBpm(s.c_str());
	}

	u::log::print("[glue::setBpm_] Bpm changed to %s (real=%f)\n", s.c_str(), m::clock::getBpm());
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


Timer::Timer(const m::model::Clock& c)
: bpm             (c.bpm)
, beats           (c.beats)
, bars            (c.bars)
, quantize        (c.quantize)
, isUsingJack     (m::kernelAudio::getAPI() == G_SYS_API_JACK)
, isRecordingInput(m::recManager::isRecordingInput())
{
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


IO::IO(const m::Channel& out, const m::Channel& in, const m::model::Mixer& m)
: masterOutVol       (out.state->volume.load())
, masterInVol        (in.state->volume.load())
#ifdef WITH_VST
, masterOutHasPlugins(out.pluginIds.size() > 0)
, masterInHasPlugins (in.pluginIds.size() > 0)
#endif
, inToOut            (m.inToOut)
{
}


/* -------------------------------------------------------------------------- */


float IO::a_getMasterOutPeak()
{
	return m::mixer::peakOut.load();
}


float IO::a_getMasterInPeak()
{
	return m::mixer::peakIn.load();
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


Timer getTimer()
{
	namespace mm = m::model;
	
	mm::ClockLock c(mm::clock);
	return Timer(*mm::clock.get());
}


/* -------------------------------------------------------------------------- */


IO getIO()
{
	namespace mm = m::model;

	mm::ChannelsLock cl(mm::channels);
	mm::MixerLock    ml(mm::mixer);

	return IO(mm::get(mm::channels, m::mixer::MASTER_OUT_CHANNEL_ID), 
	          mm::get(mm::channels, m::mixer::MASTER_IN_CHANNEL_ID),
			  *mm::mixer.get());
}


/* -------------------------------------------------------------------------- */


void setBpm(const char* v1, const char* v2)
{
	/* Never change this stuff while recording audio. */

	if (m::recManager::isRecordingInput())
		return;

	/* A value such as atof("120.1") will never be 120.1 but 120.0999999, because 
	of the rounding error. So we pass the actual "wrong" value to mixer and we show 
	the nice looking (but fake) one to the GUI. 
	On Linux, let Jack handle the bpm change if it's on. */

	float       f = std::atof(v1) + (std::atof(v2)/10);
	std::string s = std::string(v1) + "." + std::string(v2);

#if defined(G_OS_LINUX) || defined(G_OS_FREEBSD)
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
	/* Never change this stuff while recording audio. */

	if (m::recManager::isRecordingInput())
		return;

	m::clock::setBeats(beats, bars);
	m::mixer::allocRecBuffer(m::clock::getFramesInLoop());

	G_MainWin->mainTimer->setMeter(m::clock::getBeats(), m::clock::getBars());
	u::gui::refreshActionEditor();  // in case the action editor is open
}


/* -------------------------------------------------------------------------- */


void quantize(int val)
{
	m::clock::setQuantize(val);
}


/* -------------------------------------------------------------------------- */


void clearAllSamples()
{
	if (!v::gdConfirmWin("Warning", "Free all Sample channels: are you sure?"))
		return;
	G_MainWin->delSubWindow(WID_SAMPLE_EDITOR);
	m::clock::setStatus(ClockStatus::STOPPED);
	m::mh::freeAllChannels();
	m::recorderHandler::clearAllActions();
}


/* -------------------------------------------------------------------------- */


void clearAllActions()
{
	if (!v::gdConfirmWin("Warning", "Clear all actions: are you sure?"))
		return;
	G_MainWin->delSubWindow(WID_ACTION_EDITOR);
	m::recorderHandler::clearAllActions();
}


/* -------------------------------------------------------------------------- */


void setInToOut(bool v)
{
	m::mh::setInToOut(v);
}


/* -------------------------------------------------------------------------- */


void toggleRecOnSignal()
{
	/* Can't set RecTriggerMode::SIGNAL while sequencer is running, in order
	to prevent mistakes while live recording. */
		
	if (m::conf::conf.recTriggerMode == RecTriggerMode::NORMAL && m::clock::isRunning())
		return;
	m::conf::conf.recTriggerMode = m::conf::conf.recTriggerMode == RecTriggerMode::NORMAL ? RecTriggerMode::SIGNAL : RecTriggerMode::NORMAL;
}


/* -------------------------------------------------------------------------- */


void closeProject(bool createColumns)
{
	if (!v::gdConfirmWin("Warning", "Close project: are you sure?"))
		return;
	m::init::reset();
	m::mixer::enable();
}
}}} // giada::c::main::
