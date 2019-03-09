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


#include <FL/fl_draw.H>
#include "core/channels/channel.h"
#include "core/const.h"
#include "core/recorder.h"
#include "utils/string.h"
#include "channelButton.h"


namespace giada {
namespace v
{
geChannelButton::geChannelButton(int x, int y, int w, int h, const m::Channel* ch)
: geButton(x, y, w, h), 
  m_key   (""),
  m_ch    (ch)
{
}


/* -------------------------------------------------------------------------- */


void geChannelButton::refresh()
{
	switch (m_ch->status) {
		case ChannelStatus::OFF:
		case ChannelStatus::EMPTY:
			setDefaultMode(); break;
		case ChannelStatus::PLAY:
			setPlayMode(); break;
		case ChannelStatus::ENDING:
			setEndingMode(); break;
		default: break;
	}

	switch (m_ch->recStatus) {
		case ChannelStatus::ENDING:
			setEndingMode(); break;
		default: break;
	}
}


/* -------------------------------------------------------------------------- */


void geChannelButton::setKey(int k)
{
	m_key = k == 0 ? "" : std::string(1, k);
	redraw();
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

}} // giada::v::
