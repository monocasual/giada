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


#include "../../../core/graphics.h"
#include "../../../glue/transport.h"
#include "../../../glue/io.h"
#include "../../elems/basics/button.h"
#include "mainTransport.h"


namespace giada {
namespace v
{
geMainTransport::geMainTransport(int x, int y)
	: Fl_Group(x, y, 158, 25)
{
	begin();

	rewind      = new geButton(x, y, 25, 25, "", rewindOff_xpm, rewindOn_xpm);
	play        = new geButton(rewind->x()+rewind->w()+4, y, 25, 25, "", play_xpm, pause_xpm);
	recAction   = new geButton(play->x()+play->w()+8, y, 25, 25, "", recOff_xpm, recOn_xpm);
	recInput    = new geButton(recAction->x()+recAction->w()+4, y, 25, 25, "", inputRecOff_xpm, inputRecOn_xpm);
	recOnSignal = new geButton(recInput->x()+recInput->w()+4, y+5, 15, 15, "");
	metronome   = new geButton(recOnSignal->x()+recOnSignal->w()+8, y+5, 15, 15, "", metronomeOff_xpm, metronomeOn_xpm);

	end();

	resizable(nullptr);   // don't resize any widget

	rewind->callback([](Fl_Widget* w, void* v) { 
		c::transport::rewindSeq(/*gui=*/true);
	});

	play->callback([](Fl_Widget* w, void* v) { 
		c::transport::startStopSeq(/*gui=*/true);
	});
	play->type(FL_TOGGLE_BUTTON);

	recAction->callback([](Fl_Widget* w, void* v) { 
		c::io::toggleActionRec(/*gui=*/true);
	});
	recAction->type(FL_TOGGLE_BUTTON);

	recInput->callback([](Fl_Widget* w, void* v) { 
		c::io::toggleInputRec(/*gui=*/true);
	});
	recInput->type(FL_TOGGLE_BUTTON);

	recOnSignal->callback([](Fl_Widget* w, void* v) { 
		c::io::toggleRecOnSignal(/*gui=*/true); 
	});
	recOnSignal->type(FL_TOGGLE_BUTTON);

	metronome->callback([](Fl_Widget* w, void* v) {
		c::transport::toggleMetronome(/*gui=*/true);
	});
	metronome->type(FL_TOGGLE_BUTTON);
}


/* -------------------------------------------------------------------------- */


void geMainTransport::updatePlay(int v)
{
	play->value(v);
	play->redraw();
}


/* -------------------------------------------------------------------------- */


void geMainTransport::updateMetronome(int v)
{
	metronome->value(v);
	metronome->redraw();
}


/* -------------------------------------------------------------------------- */


void geMainTransport::updateRecInput(int v)
{
	recInput->value(v);
	recInput->redraw();
}


/* -------------------------------------------------------------------------- */


void geMainTransport::updateRecAction(int v)
{
	recAction->value(v);
	recAction->redraw();
}

}} // giada::v::