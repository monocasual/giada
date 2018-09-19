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
#include "../../../core/const.h"
#include "../../../core/sampleChannel.h"
#include "sampleAction.h"


namespace giada {
namespace v
{
geSampleAction::geSampleAction(Pixel X, Pixel Y, Pixel W, Pixel H, 
	const SampleChannel* ch, m::recorder::action a1, m::recorder::action a2)
: geBaseAction(X, Y, W, H, ch->mode == ChannelMode::SINGLE_PRESS, a1, a2),
  m_ch        (ch)
{
}


/* -------------------------------------------------------------------------- */


void geSampleAction::draw()
{
	Fl_Color color = hovered ? G_COLOR_LIGHT_2 : G_COLOR_LIGHT_1; 

	if (m_ch->mode == ChannelMode::SINGLE_PRESS) {
		fl_rectf(x(), y(), w(), h(), color);
	}
	else {
		if (a1.type == G_ACTION_KILL)
			fl_rect(x(), y(), MIN_WIDTH, h(), color);
		else {
			fl_rectf(x(), y(), MIN_WIDTH, h(), color);
			if (a1.type == G_ACTION_KEYPRESS)
				fl_rectf(x()+3, y()+h()-11, w()-6, 8, G_COLOR_GREY_4);
			else
			if (a1.type == G_ACTION_KEYREL)
				fl_rectf(x()+3, y()+3, w()-6, 8, G_COLOR_GREY_4);
		}
	}
}
}} // giada::v::