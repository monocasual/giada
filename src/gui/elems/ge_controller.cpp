/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 * ge_controller
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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


#include "../../core/graphics.h"
#include "../../glue/main.h"
#include "../../glue/io.h"
#include "ge_mixed.h"
#include "ge_controller.h"


gController::gController(int x, int y)
	: Fl_Group(x, y, 131, 25)
{
	begin();

	rewind    = new gClick(x,  y, 25, 25, "", rewindOff_xpm, rewindOn_xpm);
	play      = new gClick(rewind->x()+rewind->w()+4, y, 25, 25, "", play_xpm, pause_xpm);
	recAction = new gClick(play->x()+play->w()+4, y, 25, 25, "", recOff_xpm, recOn_xpm);
	recInput  = new gClick(recAction->x()+recAction->w()+4, y, 25, 25, "", inputRecOff_xpm, inputRecOn_xpm);
	metronome = new gClick(recInput->x()+recInput->w()+4, y+10, 15, 15, "", metronomeOff_xpm, metronomeOn_xpm);

	end();

	resizable(NULL);   // don't resize any widget

	rewind->callback(cb_rewind, (void*)this);

	play->callback(cb_play);
	play->type(FL_TOGGLE_BUTTON);

	recAction->callback(cb_recAction, (void*)this);
	recAction->type(FL_TOGGLE_BUTTON);

	recInput->callback(cb_recInput, (void*)this);
	recInput->type(FL_TOGGLE_BUTTON);

	metronome->callback(cb_metronome);
	metronome->type(FL_TOGGLE_BUTTON);
}


/* -------------------------------------------------------------------------- */


void gController::cb_rewind   (Fl_Widget *v, void *p) { ((gController*)p)->__cb_rewind(); }
void gController::cb_play     (Fl_Widget *v, void *p) { ((gController*)p)->__cb_play(); }
void gController::cb_recAction(Fl_Widget *v, void *p) { ((gController*)p)->__cb_recAction(); }
void gController::cb_recInput (Fl_Widget *v, void *p) { ((gController*)p)->__cb_recInput(); }
void gController::cb_metronome(Fl_Widget *v, void *p) { ((gController*)p)->__cb_metronome(); }


/* -------------------------------------------------------------------------- */


void gController::__cb_rewind()
{
	glue_rewindSeq();
}


/* -------------------------------------------------------------------------- */


void gController::__cb_play()
{
	glue_startStopSeq();
}


/* -------------------------------------------------------------------------- */


void gController::__cb_recAction()
{
	glue_startStopActionRec(true);
}


/* -------------------------------------------------------------------------- */


void gController::__cb_recInput()
{
	glue_startStopInputRec();
}


/* -------------------------------------------------------------------------- */


void gController::__cb_metronome()
{
	glue_startStopMetronome();
}


/* -------------------------------------------------------------------------- */


void gController::updatePlay(int v)
{
	play->value(v);
	play->redraw();
}


/* -------------------------------------------------------------------------- */


void gController::updateMetronome(int v)
{
	metronome->value(v);
	metronome->redraw();
}


/* -------------------------------------------------------------------------- */


void gController::updateRecInput(int v)
{
	recInput->value(v);
	recInput->redraw();
}


/* -------------------------------------------------------------------------- */


void gController::updateRecAction(int v)
{
	recAction->value(v);
	recAction->redraw();
}
