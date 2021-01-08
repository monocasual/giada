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


#include <FL/fl_draw.H>
#include "core/const.h"
#include "glue/channel.h"
#include "channelStatus.h"


namespace giada {
namespace v
{
geChannelStatus::geChannelStatus(int x, int y, int w, int h, c::channel::Data& d)
: Fl_Box   (x, y, w, h)
, m_channel(d)
{
}


/* -------------------------------------------------------------------------- */


void geChannelStatus::draw()
{
	fl_rect(x(), y(), w(), h(), G_COLOR_GREY_4);              // reset border
	fl_rectf(x()+1, y()+1, w()-2, h()-2, G_COLOR_GREY_2);     // reset background

	ChannelStatus playStatus = m_channel.getPlayStatus();
	ChannelStatus recStatus  = m_channel.getRecStatus();
	Pixel         pos        = 0;

	if (playStatus == ChannelStatus::WAIT    || 
	    playStatus == ChannelStatus::ENDING  ||
	    recStatus  == ChannelStatus::WAIT    || 
	    recStatus  == ChannelStatus::ENDING)
	{
		fl_rect(x(), y(), w(), h(), G_COLOR_LIGHT_1);
	}
	else
	if (playStatus == ChannelStatus::PLAY) {
		/* Equation for the progress bar: 
		((chanTracker - chanStart) * w()) / (chanEnd - chanStart). */
		Frame tracker = m_channel.sample->getTracker();
		Frame begin   = m_channel.sample->getBegin();
		Frame end     = m_channel.sample->getEnd();
		pos = ((tracker - begin) * (w() - 1)) / ((end - begin));
		fl_rect(x(), y(), w(), h(), G_COLOR_LIGHT_1);
	}
	else
		fl_rectf(x()+1, y()+1, w()-2, h()-2, G_COLOR_GREY_2);  // status empty

	if (pos != 0)
		fl_rectf(x()+1, y()+1, pos, h()-2, G_COLOR_LIGHT_1);
}

}} // giada::v::
