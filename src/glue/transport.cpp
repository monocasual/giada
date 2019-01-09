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
#include "../gui/elems/mainWindow/mainTransport.h"
#include "../gui/dialogs/gd_mainWindow.h"
#include "../core/clock.h"
#include "../core/conf.h"
#include "../core/const.h"
#include "../core/kernelAudio.h"
#include "../core/kernelMidi.h"
#include "../core/mixerHandler.h"
#include "../core/mixer.h"
#include "../core/recorder.h"
#include "transport.h"


extern gdMainWindow* G_MainWin;
 

using namespace giada::m;


namespace giada {
namespace c {
namespace transport 
{
void startStopSeq(bool gui)
{
	clock::isRunning() ? stopSeq(gui) : startSeq(gui);
}


/* -------------------------------------------------------------------------- */


void startSeq(bool gui)
{
	clock::start();

#ifdef __linux__
	kernelAudio::jackStart();
#endif

	if (!gui) {
		Fl::lock();
		G_MainWin->mainTransport->updatePlay(1);
		Fl::unlock();
  }
}


/* -------------------------------------------------------------------------- */


void stopSeq(bool gui)
{
	mh::stopSequencer();

#ifdef __linux__
	kernelAudio::jackStop();
#endif

	/* What to do if we stop the sequencer and some action recs are active?
	Deactivate the button and delete any 'rec on' status. */

	if (recorder::isActive()) {
		recorder::disable();
		Fl::lock();
		G_MainWin->mainTransport->updateRecAction(0);
		Fl::unlock();
	}

	/* If input recs are active (who knows why) we must deactivate them. One 
	might stop the sequencer while an input rec is running. */

	if (mixer::recording) {
		mh::stopInputRec();
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


void rewindSeq(bool gui, bool notifyJack)
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


void toggleMetronome(bool gui)
{
	mixer::toggleMetronome();
	if (!gui) {
		Fl::lock();
		G_MainWin->mainTransport->updateMetronome(mixer::isMetronomeOn());
		Fl::unlock();
	}
}

}}} // giada::c::transport::