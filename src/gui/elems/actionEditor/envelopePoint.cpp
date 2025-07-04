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

#include "src/gui/elems/actionEditor/envelopePoint.h"
#include "src/gui/const.h"
#include <FL/fl_draw.H>

namespace giada::v
{
geEnvelopePoint::geEnvelopePoint(Pixel X, Pixel Y, m::Action a)
: geBaseAction(X, Y, SIDE, SIDE, /*resizable=*/false, a, {})
{
}

/* -------------------------------------------------------------------------- */

void geEnvelopePoint::draw()
{
	fl_color(hovered ? G_COLOR_LIGHT_2 : G_COLOR_LIGHT_1);
	fl_pie(x(), y(), w(), h(), 0, 360);
}
} // namespace giada::v