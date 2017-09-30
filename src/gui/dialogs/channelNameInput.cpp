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

/*
#include <cstring>
#include "../../core/conf.h"
#include "../../core/mixer.h"
#include "../../core/clock.h"
#include "../../glue/main.h"
#include "../../utils/string.h"*/
#include "../../utils/gui.h"
#include "../../core/const.h"
#include "../../core/channel.h"
#include "../elems/basics/button.h"
#include "../elems/basics/input.h"
#include "channelNameInput.h"
#include "gd_mainWindow.h"


extern gdMainWindow* mainWin;


//using std::vector;
//using std::string;
//using namespace giada::m;


gdChannelNameInput::gdChannelNameInput(Channel* ch)
: gdWindow(400, 36, "New channel name"),
  m_ch    (ch)
{
	//if (conf::bpmX)
	//	resize(conf::bpmX, conf::bpmY, w(), h());

	set_modal();

	m_name = new geInput(8,  8, 100, G_GUI_UNIT);
	m_ok 	= new geButton(m_name->x()+m_name->w()+4, 8, 70, G_GUI_UNIT, "Ok");
	end();

	m_name->value(m_ch->getName().c_str());

	m_ok->shortcut(FL_Enter);
	m_ok->callback(cb_update, (void*)this);

	gu_setFavicon(this);
	setId(WID_SAMPLE_NAME);
	show();
}


/* -------------------------------------------------------------------------- */


gdChannelNameInput::~gdChannelNameInput()
{
	//conf::bpmX = x();
	//conf::bpmY = y();
}


/* -------------------------------------------------------------------------- */


void gdChannelNameInput::cb_update(Fl_Widget* w, void* p) { ((gdChannelNameInput*)p)->cb_update(); }


/* -------------------------------------------------------------------------- */


void gdChannelNameInput::cb_update()
{
	/*
	if (strcmp(input_a->value(), "") == 0)
		return;
	glue_setBpm(input_a->value(), input_b->value());*/
	do_callback();
}
