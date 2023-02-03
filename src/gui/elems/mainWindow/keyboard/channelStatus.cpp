/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "gui/elems/mainWindow/keyboard/channelStatus.h"
#include "core/const.h"
#include "glue/channel.h"
#include "gui/drawing.h"
#include "utils/math.h"
#include <FL/fl_draw.H>

namespace giada::v
{
geChannelStatus::geChannelStatus(int x, int y, int w, int h, c::channel::Data& d)
: Fl_Box(x, y, w, h)
, m_channel(d)
{
}

/* -------------------------------------------------------------------------- */

void geChannelStatus::draw()
{
	const ChannelStatus playStatus = m_channel.getPlayStatus();
	const Frame         tracker    = m_channel.sample->getTracker();
	const Frame         begin      = m_channel.sample->begin;
	const Frame         end        = m_channel.sample->end;
	const Pixel         pos        = u::math::map(tracker, begin, end, 0, w());

	const geompp::Rect<int> bounds(x(), y(), w(), h());

	drawRectf(bounds, G_COLOR_GREY_2); // reset background

	if (playStatus == ChannelStatus::PLAY ||
	    playStatus == ChannelStatus::WAIT ||
	    playStatus == ChannelStatus::ENDING)
	{
		drawRectf(bounds.withW(pos), G_COLOR_LIGHT_1);
		drawRect(bounds, G_COLOR_LIGHT_1);
	}
	else
	{
		drawRectf(bounds.withW(pos), G_COLOR_GREY_4);
		drawRect(bounds, G_COLOR_GREY_4);
	}
}
} // namespace giada::v