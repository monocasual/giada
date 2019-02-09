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


#include <FL/Fl.H>
#include "../gui/dialogs/mainWindow.h"
#include "../gui/dialogs/warnings.h"
#include "../gui/elems/basics/button.h"
#include "../gui/elems/mainWindow/mainTransport.h"
#include "../gui/elems/mainWindow/mainTimer.h"
#include "../gui/elems/mainWindow/keyboard/keyboard.h"
#include "../gui/elems/mainWindow/keyboard/channel.h"
#include "../gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "../utils/gui.h"
#include "../utils/log.h"
#include "../utils/math.h"
#include "../core/recorder.h"
#include "../core/conf.h"
#include "../core/recManager.h"
#include "../core/kernelAudio.h"
#include "../core/channel.h"
#include "../core/mixer.h"
#include "../core/mixerHandler.h"
#include "../core/wave.h"
#include "../core/midiDispatcher.h"
#include "../core/channel.h"
#include "../core/clock.h"
#include "../core/sampleChannel.h"
#include "../core/midiChannel.h"
#include "../core/recorderHandler.h"
#include "main.h"
#include "channel.h"
#include "transport.h"
#include "io.h"


extern gdMainWindow* G_MainWin;


namespace giada {
namespace c {
namespace io 
{
void keyPress(m::Channel* ch, bool ctrl, bool shift, int velocity)
{
	/* Everything occurs on frame 0 here: they are all user-generated events. */
	if (ctrl)
		c::channel::toggleMute(ch);
	else
	if (shift) {
		if (ch->recordKill())
			ch->kill(0);
	}
	else {
		if (ch->recordStart(m::clock::canQuantize()))
			ch->start(0, m::clock::canQuantize(), velocity);
	}
}


/* -------------------------------------------------------------------------- */


void keyRelease(m::Channel* ch, bool ctrl, bool shift)
{
	if (!ctrl && !shift) {
		ch->recordStop();
		ch->stop();
	}
}


/* -------------------------------------------------------------------------- */


void toggleActionRec(bool gui)
{
	m::recManager::isActive() ? stopActionRec(gui) : startActionRec(gui);
}


/* -------------------------------------------------------------------------- */


void startActionRec(bool gui)
{
	RecTriggerMode triggerMode = static_cast<RecTriggerMode>(m::conf::recTriggerMode);

	if (!m::recManager::startActionRec(triggerMode))
		return;
	if (!gui) Fl::lock();
	G_MainWin->mainTransport->updatePlay(m::clock::isRunning());
	G_MainWin->mainTransport->updateRecAction(1);
	if (!gui) Fl::unlock();
}


/* -------------------------------------------------------------------------- */


void stopActionRec(bool gui)
{
	m::recManager::stopActionRec();

	if (!gui) Fl::lock();
	G_MainWin->mainTransport->updateRecAction(0);
	for (m::Channel* ch : m::mixer::channels)
		if (ch->type == ChannelType::SAMPLE)
			G_MainWin->keyboard->setChannelWithActions(static_cast<geSampleChannel*>(ch->guiChannel));
	if (!gui) Fl::unlock();

	u::gui::refreshActionEditor();  // in case it's open
}


/* -------------------------------------------------------------------------- */


void toggleInputRec(bool gui)
{
	if (m::recManager::isActive())
		stopInputRec(gui);
	else
	if (!startInputRec(gui))
		gdAlert("No channels armed/available for audio recording.");
}


/* -------------------------------------------------------------------------- */


bool startInputRec(bool gui)
{
	RecTriggerMode triggerMode = static_cast<RecTriggerMode>(m::conf::recTriggerMode);

	if (!m::recManager::startInputRec(triggerMode)) {
		if (!gui) Fl::lock();
		G_MainWin->mainTransport->updateRecInput(0);  // set it off, anyway
		if (!gui) Fl::unlock();
		return false;
	}	

	if (!gui) Fl::lock();
	G_MainWin->mainTransport->updatePlay(m::clock::isRunning());
	G_MainWin->mainTransport->updateRecInput(1);
	G_MainWin->mainTimer->setLock(true);
	if (!gui) Fl::unlock();

	return true;
}


/* -------------------------------------------------------------------------- */


void stopInputRec(bool gui)
{
	m::recManager::stopInputRec();

	if (!gui) Fl::lock();
	G_MainWin->mainTransport->updateRecInput(0);
	G_MainWin->mainTimer->setLock(false);
	/* Update sample name inside sample channels' main button. This is useless 
	for MIDI channels, but let's do it anyway. */
	for (const m::Channel* ch : m::mixer::channels)
		ch->guiChannel->update();
	if (!gui) Fl::unlock();
}

}}} // giada::c::io::