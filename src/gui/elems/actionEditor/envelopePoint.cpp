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
#include "core/const.h"
#include "envelopePoint.h"


namespace giada {
namespace v
{
geEnvelopePoint::geEnvelopePoint(Pixel X, Pixel Y, m::Action a)
	: geBaseAction(X, Y, SIDE, SIDE, /*resizable=*/false, a, {})
{
}


/* -------------------------------------------------------------------------- */


void geEnvelopePoint::draw()
{
	fl_rectf(x(), y(), w(), h(), hovered ? G_COLOR_LIGHT_2 : G_COLOR_LIGHT_1);
}
}} // giada::v::