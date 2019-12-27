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


#include "core/graphics.h"
#include "core/conf.h"
#include "core/clock.h"
#include "core/mixer.h"
#include "core/mixerHandler.h"
#include "core/recManager.h"
#include "core/conf.h"
#include "core/const.h"
#include "glue/main.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/statusButton.h"
#include "mainTransport.h"


namespace giada {
namespace v
{
geMainTransport::geMainTransport(int x, int y)
: Fl_Pack(x, y, 200, 25)
{
	type(Fl_Pack::HORIZONTAL);
	spacing(G_GUI_INNER_MARGIN);

	rewind         = new geButton      (0, 0, 25, 25, "", rewindOff_xpm, rewindOn_xpm);
	play           = new geStatusButton(0, 0, 25, 25, play_xpm, pause_xpm);
	                 new geBox         (0, 0, 10, 25);
	recTriggerMode = new geButton      (0, 0, 15, 25, "", recTriggerModeOff_xpm, recTriggerModeOn_xpm);
	recAction      = new geStatusButton(0, 0, 25, 25, recOff_xpm, recOn_xpm);
	recInput       = new geStatusButton(0, 0, 25, 25, inputRecOff_xpm, inputRecOn_xpm);
	                 new geBox         (0, 0, 10, 25);
	metronome      = new geStatusButton(0, 0, 15, 25, metronomeOff_xpm, metronomeOn_xpm);

	rewind->callback([](Fl_Widget* w, void* v) { 
		m::mh::rewindSequencer();
	});

	play->callback([](Fl_Widget* w, void* v) { 
		m::mh::toggleSequencer();
	});

	recAction->callback([](Fl_Widget* w, void* v) { 
		c::main::toggleActionRec();
	});

	recInput->callback([](Fl_Widget* w, void* v) { 
		c::main::toggleInputRec();
	});

	recTriggerMode->value(static_cast<int>(m::conf::conf.recTriggerMode));
	recTriggerMode->type(FL_TOGGLE_BUTTON);
	recTriggerMode->callback([](Fl_Widget* w, void* v) { 
		m::conf::conf.recTriggerMode = static_cast<RecTriggerMode>(static_cast<geButton*>(w)->value());
	});

	metronome->type(FL_TOGGLE_BUTTON);
	metronome->callback([](Fl_Widget* w, void* v) {
		m::mixer::toggleMetronome();
	});
}


/* -------------------------------------------------------------------------- */


void geMainTransport::refresh()
{
	play->setStatus(m::clock::isRunning());
	recAction->setStatus(m::recManager::isRecordingAction());
	recInput->setStatus(m::recManager::isRecordingInput());
	metronome->setStatus(m::mixer::isMetronomeOn());
}

}} // giada::v::
