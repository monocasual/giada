/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/gui/elems/mainWindow/keyboard/channelProgress.h"
#include "src/glue/channel.h"
#include "src/gui/const.h"
#include "src/gui/drawing.h"
#include "src/gui/types.h"
#include "src/utils/math.h"

namespace giada::v
{
geChannelProgress::geChannelProgress(int x, int y, int w, int h, c::channel::Data& d)
: Fl_Box(x, y, w, h)
, m_channel(d)
{
}

/* -------------------------------------------------------------------------- */

void geChannelProgress::draw()
{
	const Frame tracker = m_channel.sample->getTracker();
	const auto  range   = m_channel.sample->range;
	const Pixel pos     = range.isValid() ? u::math::map(std::max(tracker, range.a), range.a, range.b, 0, w()) : 0;

	const geompp::Rect<int> bounds(x(), y(), w(), h());

	drawRectf(bounds, G_COLOR_GREY_2); // reset background

	if (m_channel.isActive())
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