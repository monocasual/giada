/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#include <cstring>
#include "../../utils/gui.h"
#include "../../utils/string.h"
#include "../../core/mixer.h"
#include "../../core/clock.h"
#include "../../core/conf.h"
#include "../../core/const.h"
#include "../../glue/main.h"
#include "../elems/basics/input.h"
#include "../elems/basics/button.h"
#include "../elems/basics/check.h"
#include "beatsInput.h"
#include "gd_mainWindow.h"


extern gdMainWindow* mainWin;


using namespace giada::m;


gdBeatsInput::gdBeatsInput()
	: gdWindow(180, 60, "Beats")
{
	if (conf::beatsX)
		resize(conf::beatsX, conf::beatsY, w(), h());

	set_modal();

	beats     = new geInput(8,  8,  43, G_GUI_UNIT);
	bars      = new geInput(beats->x()+beats->w()+4, 8,  43, G_GUI_UNIT);
	ok 		    = new geButton(bars->x()+bars->w()+4, 8,  70, G_GUI_UNIT, "Ok");
	resizeRec = new geCheck(8,  40, 12, 12, "resize recorded actions");
	end();

	beats->maximum_size(2);
	beats->value(gu_toString(clock::getBeats()).c_str());
	beats->type(FL_INT_INPUT);
	
	bars->maximum_size(2);
	bars->value(gu_toString(clock::getBars()).c_str());
	bars->type(FL_INT_INPUT);
	
	ok->shortcut(FL_Enter);
	ok->callback(cb_update, (void*)this);
	
	resizeRec->value(conf::resizeRecordings);

	gu_setFavicon(this);
	setId(WID_BEATS);
	show();
}


/* -------------------------------------------------------------------------- */


gdBeatsInput::~gdBeatsInput()
{
	conf::beatsX = x();
	conf::beatsY = y();
	conf::resizeRecordings = resizeRec->value();
}


/* -------------------------------------------------------------------------- */


void gdBeatsInput::cb_update(Fl_Widget* w, void* p) { ((gdBeatsInput*)p)->cb_update(); }


/* -------------------------------------------------------------------------- */


void gdBeatsInput::cb_update()
{
	if (!strcmp(beats->value(), "") || !strcmp(bars->value(), ""))
		return;
	glue_setBeats(atoi(beats->value()), atoi(bars->value()), resizeRec->value());
	do_callback();
}
