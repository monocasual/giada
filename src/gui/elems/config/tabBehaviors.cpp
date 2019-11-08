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


#include <FL/Fl_Pack.H>
#include "core/const.h"
#include "core/conf.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/radio.h"
#include "gui/elems/basics/check.h"
#include "tabBehaviors.h"


namespace giada {
namespace v
{
geTabBehaviors::geTabBehaviors(int X, int Y, int W, int H)
: Fl_Group(X, Y, W, H, "Behaviors")
{
	begin();

	Fl_Group* radioGrp_1 = new Fl_Group(x(), y()+10, w(), 70); // radio group for the mutex
		new geBox(x(), y()+10, 70, 25, "When a channel with recorded actions is halted:", FL_ALIGN_LEFT);
		recsStopOnChanHalt_1 = new geRadio(x()+25, y()+35, 280, 20, "stop it immediately");
		recsStopOnChanHalt_0 = new geRadio(x()+25, y()+60, 280, 20, "play it until finished");
	radioGrp_1->end();

	Fl_Group* radioGrp_2 = new Fl_Group(x(), radioGrp_1->y()+radioGrp_1->h(), w(), 70); // radio group for the mutex
		new geBox(x(), y()+80, 70, 25, "When the sequencer is halted:", FL_ALIGN_LEFT);
		chansStopOnSeqHalt_1 = new geRadio(x()+25, y()+105, 280, 20, "stop immediately all dynamic channels");
		chansStopOnSeqHalt_0 = new geRadio(x()+25, y()+130, 280, 20, "play all dynamic channels until finished");
	radioGrp_2->end();

	treatRecsAsLoops      = new geCheck(x(), radioGrp_2->y()+radioGrp_2->h() + 15, 280, 20, "Treat one shot channels with actions as loops");
	inputMonitorDefaultOn = new geCheck(x(), treatRecsAsLoops->y()+treatRecsAsLoops->h() + 5, 280, 20, "New sample channels have input monitor on by default");

	end();

	labelsize(G_GUI_FONT_SIZE_BASE);
	selection_color(G_COLOR_GREY_4);

	m::conf::recsStopOnChanHalt == 1 ? recsStopOnChanHalt_1->value(1) : recsStopOnChanHalt_0->value(1);
	m::conf::chansStopOnSeqHalt == 1 ? chansStopOnSeqHalt_1->value(1) : chansStopOnSeqHalt_0->value(1);
	treatRecsAsLoops->value(m::conf::treatRecsAsLoops);
	inputMonitorDefaultOn->value(m::conf::inputMonitorDefaultOn);

	recsStopOnChanHalt_1->callback(cb_radio_mutex, (void*)this);
	recsStopOnChanHalt_0->callback(cb_radio_mutex, (void*)this);
	chansStopOnSeqHalt_1->callback(cb_radio_mutex, (void*)this);
	chansStopOnSeqHalt_0->callback(cb_radio_mutex, (void*)this);
}


/* -------------------------------------------------------------------------- */


void geTabBehaviors::cb_radio_mutex(Fl_Widget* w, void* p) 
{ 
	static_cast<geTabBehaviors*>(p)->cb_radio_mutex(w); 
}


/* -------------------------------------------------------------------------- */


void geTabBehaviors::cb_radio_mutex(Fl_Widget* w)
{
	static_cast<Fl_Button*>(w)->type(FL_RADIO_BUTTON);
}


/* -------------------------------------------------------------------------- */


void geTabBehaviors::save()
{
	m::conf::recsStopOnChanHalt = recsStopOnChanHalt_1->value() == 1 ? 1 : 0;
	m::conf::chansStopOnSeqHalt = chansStopOnSeqHalt_1->value() == 1 ? 1 : 0;
	m::conf::treatRecsAsLoops = treatRecsAsLoops->value() == 1 ? 1 : 0;
	m::conf::inputMonitorDefaultOn = inputMonitorDefaultOn->value() == 1 ? 1 : 0;
}
}} // giada::v::