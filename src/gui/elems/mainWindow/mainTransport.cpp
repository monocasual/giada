/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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


#include "core/graphics.h"
#include "core/conf.h"
#include "core/clock.h"
#include "core/sequencer.h"
#include "core/mixer.h"
#include "core/mixerHandler.h"
#include "core/recManager.h"
#include "core/conf.h"
#include "core/const.h"
#include "glue/main.h"
#include "glue/events.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/statusButton.h"
#include "mainTransport.h"


namespace giada {
namespace v
{
geMainTransport::geMainTransport(int x, int y)
: gePack(x, y, Direction::HORIZONTAL)
{
	rewind         = new geButton      (0, 0, 25, 25, "", rewindOff_xpm, rewindOn_xpm);
	play           = new geStatusButton(0, 0, 25, 25, play_xpm, pause_xpm);
	spacer1        = new geBox         (0, 0, 10, 25);
	recTriggerMode = new geStatusButton(0, 0, 15, 25, recTriggerModeOff_xpm, recTriggerModeOn_xpm);
	recAction      = new geStatusButton(0, 0, 25, 25, recOff_xpm, recOn_xpm);
	recInput       = new geStatusButton(0, 0, 25, 25, inputRecOff_xpm, inputRecOn_xpm);
	spacer2        = new geBox         (0, 0, 10, 25);
	metronome      = new geStatusButton(0, 0, 15, 25, metronomeOff_xpm, metronomeOn_xpm);
	add(rewind);
	add(play);
	add(spacer1);
	add(recTriggerMode);
	add(recAction);
	add(recInput);
	add(spacer2);
	add(metronome);
	
	rewind->callback([](Fl_Widget* /*w*/, void* /*v*/) { 
		c::events::rewindSequencer(Thread::MAIN);
	});

	play->callback([](Fl_Widget* /*w*/, void* /*v*/) { 
		c::events::toggleSequencer(Thread::MAIN);
	});

	recAction->callback([](Fl_Widget* /*w*/, void* /*v*/) { 
		c::events::toggleActionRecording();
	});

	recInput->callback([](Fl_Widget* /*w*/, void* /*v*/) { 
		c::events::toggleInputRecording();
	});

	recTriggerMode->callback([](Fl_Widget* /*w*/, void* /*v*/) { 
		c::main::toggleRecOnSignal();
	});

	metronome->type(FL_TOGGLE_BUTTON);
	metronome->callback([](Fl_Widget* /*w*/, void* /*v*/) {
		c::events::toggleMetronome();
	});
}


/* -------------------------------------------------------------------------- */


void geMainTransport::refresh()
{
	play->setStatus(m::clock::isRunning());
	recAction->setStatus(m::recManager::isRecordingAction());
	recInput->setStatus(m::recManager::isRecordingInput());
	metronome->setStatus(m::sequencer::isMetronomeOn());
	recTriggerMode->setStatus(m::conf::conf.recTriggerMode == RecTriggerMode::SIGNAL);
}
}} // giada::v::
