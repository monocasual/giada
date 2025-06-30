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

#include "gui/elems/midiActivity.h"
#include "gui/const.h"
#include "gui/ui.h"
#include <FL/fl_draw.H>

extern giada::v::Ui* g_ui;

namespace giada::v
{
geMidiLed::geMidiLed()
: Fl_Button(0, 0, 0, 0)
, m_decay(0) // decay > 0: led is on
{
}

/* -------------------------------------------------------------------------- */

void geMidiLed::draw()
{
	int bgColor = G_COLOR_GREY_2;
	int bdColor = G_COLOR_GREY_4;

	if (m_decay > 0) // If led is on
	{
		m_decay = (m_decay + 1) % (G_GUI_FPS / 4);

		bgColor = G_COLOR_LIGHT_2;
		bdColor = G_COLOR_LIGHT_2;
	}

	fl_rectf(x(), y(), w(), h(), bgColor); // background
	fl_rect(x(), y(), w(), h(), bdColor);  // border
}

/* -------------------------------------------------------------------------- */

void geMidiLed::lit()
{
	m_decay = 1;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geMidiActivity::geMidiActivity(bool hasOut)
: geFlex(Direction::VERTICAL, G_GUI_INNER_MARGIN)
{
	out = new geMidiLed();
	in  = new geMidiLed();
	if (hasOut)
		addWidget(out);
	addWidget(in);
	end();
}
} // namespace giada::v