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


#include <FL/Fl.H>
#include "../gui/dialogs/gd_mainWindow.h"
#include "../gui/dialogs/gd_warnings.h"
#include "../gui/elems/mainWindow/mainTransport.h"
#include "../gui/elems/mainWindow/mainTimer.h"
#include "../gui/elems/mainWindow/keyboard/keyboard.h"
#include "../gui/elems/mainWindow/keyboard/channel.h"
#include "../gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "../utils/gui.h"
#include "../utils/log.h"
#include "../utils/math.h"
#include "../core/recorder.h"
#include "../core/kernelAudio.h"
#include "../core/mixer.h"
#include "../core/mixerHandler.h"
#include "../core/wave.h"
#include "../core/channel.h"
#include "../core/clock.h"
#include "../core/sampleChannel.h"
#include "../core/midiChannel.h"
#include "main.h"
#include "channel.h"
#include "transport.h"
#include "io.h"


extern gdMainWindow* G_MainWin;


namespace giada {
namespace c     {
namespace io 
{
void keyPress(Channel* ch, bool ctrl, bool shift, int velocity)
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
			ch->start(0, m::clock::canQuantize(), 0);
	}
}


/* -------------------------------------------------------------------------- */


void keyRelease(Channel* ch, bool ctrl, bool shift)
{
	if (!ctrl && !shift) {
		ch->recordStop();
		ch->stop();
	}
}


/* -------------------------------------------------------------------------- */


void startStopActionRec(bool gui)
{
	m::recorder::active ? stopActionRec(gui) : startActionRec(gui);
}


/* -------------------------------------------------------------------------- */


void startActionRec(bool gui)
{
	using namespace giada::m;

	if (kernelAudio::getStatus() == false)
		return;

	recorder::active = true;

	if (!clock::isRunning())
		glue_startSeq(false);  // update gui

	if (!gui) {
		Fl::lock();
		G_MainWin->mainTransport->updateRecAction(1);
		Fl::unlock();
	}
}


/* -------------------------------------------------------------------------- */


void stopActionRec(bool gui)
{
	/* Stop the recorder and sort newly recorder actions. */

	m::recorder::active = false;
	m::recorder::sortActions();

	for (Channel* ch : m::mixer::channels)
	{
		if (ch->type == ChannelType::MIDI)
			continue;
		G_MainWin->keyboard->setChannelWithActions(static_cast<geSampleChannel*>(ch->guiChannel));
		if (!ch->readActions && ch->hasActions)
			c::channel::startReadingActions(ch, false);
	}

	if (!gui) {
		Fl::lock();
		G_MainWin->mainTransport->updateRecAction(0);
		Fl::unlock();
	}

	gu_refreshActionEditor();  // in case it's open
}


/* -------------------------------------------------------------------------- */


void startStopInputRec(bool gui)
{
	if (m::mixer::recording)
		stopInputRec(gui);
	else
	if (!startInputRec(gui))
		gdAlert("No channels armed/available for audio recording.");
}


/* -------------------------------------------------------------------------- */


int startInputRec(bool gui)
{
	using namespace giada::m;

	if (kernelAudio::getStatus() == false)
		return false;

	if (!mh::startInputRec()) {
		Fl::lock();
		G_MainWin->mainTransport->updateRecInput(0);  // set it off, anyway
		Fl::unlock();
		return false;
	}

	if (!clock::isRunning())
		glue_startSeq(false); // update gui anyway

	Fl::lock();
		if (!gui)
			G_MainWin->mainTransport->updateRecInput(1);
		G_MainWin->mainTimer->setLock(true);
	Fl::unlock();

	/* Update sample name inside sample channels' main button. This is useless for
	midi channel, but let's do it anyway. */

	for (Channel* ch : m::mixer::channels)
		ch->guiChannel->update();

	return true;
}


/* -------------------------------------------------------------------------- */


int stopInputRec(bool gui)
{
	using namespace giada::m;
	
	mh::stopInputRec();

	Fl::lock();
		if (!gui)
			G_MainWin->mainTransport->updateRecInput(0);
		G_MainWin->mainTimer->setLock(false);
	Fl::unlock();

	return 1;
}

}}} // giada::c::io::