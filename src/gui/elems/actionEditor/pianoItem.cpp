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

#include "pianoItem.h"
#include "core/const.h"
#include "core/midiEvent.h"
#include "src/core/actions/action.h"
#include "utils/math.h"
#include <FL/fl_draw.H>

namespace giada::v
{
gePianoItem::gePianoItem(Pixel X, Pixel Y, Pixel W, Pixel H, m::Action a1,
    m::Action a2)
: geBaseAction(X, Y, W, H, /*resizable=*/true, a1, a2)
, m_ringLoop(a2.isValid() && a1.frame > a2.frame)
, m_orphaned(!a2.isValid())
{
	m_resizable = isResizable();
}

/* -------------------------------------------------------------------------- */

bool gePianoItem::isResizable() const
{
	return !(m_ringLoop || m_orphaned);
}

/* -------------------------------------------------------------------------- */

void gePianoItem::draw()
{
	Fl_Color color = hovered ? G_COLOR_LIGHT_2 : G_COLOR_LIGHT_1;

	Pixel by = y() + 2;
	Pixel bh = h() - 3;

	if (m_orphaned)
	{
		fl_rect(x(), by, w(), bh, color);
		fl_line(x(), by, x() + w(), by + bh);
	}
	else
	{
		Pixel vh = calcVelocityH();
		if (m_ringLoop)
		{
			fl_rect(x(), by, MIN_WIDTH, bh, color);
			fl_line(x() + MIN_WIDTH, by + bh / 2, x() + w(), by + bh / 2);
			fl_rectf(x(), by + (bh - vh), MIN_WIDTH, vh, color);
		}
		else
		{
			fl_rect(x(), by, w(), bh, color);
			fl_rectf(x(), by + (bh - vh), w(), vh, color);
		}
	}
}

/* -------------------------------------------------------------------------- */

Pixel gePianoItem::calcVelocityH() const
{
	int v = a1.event.getVelocity();
	return u::math::map<int, Pixel>(v, 0, G_MAX_VELOCITY, 0, h() - 3);
}
} // namespace giada::v
