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
#include "../../../core/midiEvent.h"
#include "../../../utils/math.h"
#include "pianoItem.h"


namespace giada {
namespace v
{
gePianoItem::gePianoItem(Pixel X, Pixel Y, Pixel W, Pixel H, m::recorder::action a1,
	m::recorder::action a2)
: geBaseAction(X, Y, W, H, /*resizable=*/true, a1, a2),
  orphaned    (a2.frame == -1)
{
}


/* -------------------------------------------------------------------------- */


void gePianoItem::draw()
{
	Fl_Color color = hovered ? G_COLOR_LIGHT_2 : G_COLOR_LIGHT_1;

	Pixel by = y() + 2;
	Pixel bh = h() - 3;

	if (orphaned) {
		fl_rect(x(), by, MIN_WIDTH, bh, color);
		fl_line(x(), by, x() + MIN_WIDTH, by + bh);
	}
	else {
		Pixel vh = calcVelocityH();
		fl_rectf(x(), by + (bh - vh), w(), vh, color);
		fl_rect(x(), by, w(), bh, color);
	}
}


/* -------------------------------------------------------------------------- */


Pixel gePianoItem::calcVelocityH() const
{
	int v = m::MidiEvent(a1.iValue).getVelocity();
	return u::math::map<int, Pixel>(v, 0, G_MAX_VELOCITY, 0, h() - 3);
}
}} // giada::v::