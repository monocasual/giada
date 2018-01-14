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


#include <FL/fl_draw.H>
#include "../../../../core/const.h"
#include "../../../../utils/string.h"
#include "channelButton.h"


using std::string;


geChannelButton::geChannelButton(int x, int y, int w, int h, const char* l)
	: geButton(x, y, w, h, l), 
	  m_key   ("") 
{
}


/* -------------------------------------------------------------------------- */


void geChannelButton::setKey(const string& k)
{
	m_key = k;
}


/* -------------------------------------------------------------------------- */


void geChannelButton::setKey(int k)
{
	if (k == 0)
		m_key = "";
	else 
		m_key = static_cast<char>(k); // FIXME - What about unicode/utf-8?
}


/* -------------------------------------------------------------------------- */


void geChannelButton::draw()
{
	geButton::draw();

	if (m_key == "")
		return;

	/* draw background */

	fl_rectf(x()+1, y()+1, 18, h()-2, bgColor0);

	/* draw m_key */

	fl_color(G_COLOR_LIGHT_2);
	fl_font(FL_HELVETICA, 11);
	fl_draw(m_key.c_str(), x(), y(), 18, h(), FL_ALIGN_CENTER);
}


/* -------------------------------------------------------------------------- */


void geChannelButton::setInputRecordMode()
{
	bgColor0 = G_COLOR_RED;
}


/* -------------------------------------------------------------------------- */


void geChannelButton::setActionRecordMode()
{
	bgColor0 = G_COLOR_BLUE;
	txtColor = G_COLOR_LIGHT_2;
}


/* -------------------------------------------------------------------------- */


void geChannelButton::setDefaultMode(const char* l)
{
	bgColor0 = G_COLOR_GREY_2;
	bdColor  = G_COLOR_GREY_4;
	txtColor = G_COLOR_LIGHT_2;
	if (l)
		label(l);
}


/* -------------------------------------------------------------------------- */


void geChannelButton::setPlayMode()
{
	bgColor0 = G_COLOR_LIGHT_1;
	bdColor  = G_COLOR_LIGHT_1;
	txtColor = G_COLOR_GREY_1;
}


/* -------------------------------------------------------------------------- */


void geChannelButton::setEndingMode()
{
	bgColor0 = G_COLOR_GREY_4;
}
