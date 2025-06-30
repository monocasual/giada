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

#include "gui/elems/actionEditor/pianoRollLegend.h"
#include "gui/const.h"
#include "gui/drawing.h"
#include "gui/elems/actionEditor/pianoRoll.h"
#include <FL/fl_draw.H>

namespace giada::v
{
gePianoRollLegend::geContent::geContent(Pixel x, Pixel y, Pixel w)
: geBox(x, y, w, gePianoRoll::CELL_H * gePianoRoll::MAX_KEYS)
{
	drawOffscreen();
}
/* -------------------------------------------------------------------------- */

void gePianoRollLegend::geContent::draw()
{
	fl_copy_offscreen(x(), y(), gePianoRoll::CELL_W, h(), m_offscreen, 0, 0);
}

/* -------------------------------------------------------------------------- */

void gePianoRollLegend::geContent::drawOffscreen()
{
	m_offscreen = fl_create_offscreen(gePianoRoll::CELL_W, h());
	fl_begin_offscreen(m_offscreen);

	/* Warning: only w() and h() come from this widget, x and y coordinates are
	absolute, since we are writing in a memory chunk. */

	fl_rectf(0, 0, gePianoRoll::CELL_W, h(), G_COLOR_GREY_1_5);

	fl_line_style(FL_DASH, 0, nullptr);
	fl_color(G_COLOR_GREY_3);

	int octave = gePianoRoll::MAX_OCTAVES;

	for (int i = 1; i <= gePianoRoll::MAX_KEYS + 1; i++)
	{
		/* print key note label. C C# D D# E F F# G G# A A# B */

		std::string note = std::to_string(octave);
		switch (i % gePianoRoll::KEYS)
		{
		case (int)gePianoRoll::Notes::G:
			fl_rectf(0, i * gePianoRoll::CELL_H, gePianoRoll::CELL_W, gePianoRoll::CELL_H, G_COLOR_GREY_2);
			note += "G";
			break;
		case (int)gePianoRoll::Notes::FS:
			note += "F#";
			break;
		case (int)gePianoRoll::Notes::F:
			note += "F";
			break;
		case (int)gePianoRoll::Notes::E:
			fl_rectf(0, i * gePianoRoll::CELL_H, gePianoRoll::CELL_W, gePianoRoll::CELL_H, G_COLOR_GREY_2);
			note += "E";
			break;
		case (int)gePianoRoll::Notes::DS:
			note += "D#";
			break;
		case (int)gePianoRoll::Notes::D:
			fl_rectf(0, i * gePianoRoll::CELL_H, gePianoRoll::CELL_W, gePianoRoll::CELL_H, G_COLOR_GREY_2);
			note += "D";
			break;
		case (int)gePianoRoll::Notes::CS:
			note += "C#";
			break;
		case (int)gePianoRoll::Notes::C:
			note += "C";
			octave--;
			break;
		case (int)gePianoRoll::Notes::B:
			fl_rectf(0, i * gePianoRoll::CELL_H, gePianoRoll::CELL_W, gePianoRoll::CELL_H, G_COLOR_GREY_2);
			note += "B";
			break;
		case (int)gePianoRoll::Notes::AS:
			note += "A#";
			break;
		case (int)gePianoRoll::Notes::A:
			fl_rectf(0, i * gePianoRoll::CELL_H, gePianoRoll::CELL_W, gePianoRoll::CELL_H, G_COLOR_GREY_2);
			note += "A";
			break;
		case (int)gePianoRoll::Notes::GS:
			note += "G#";
			break;
		}

		/* Print note name */

		fl_font(FL_HELVETICA, G_GUI_FONT_SIZE_BASE);
		fl_color(G_COLOR_GREY_4);
		fl_draw(note.c_str(), 4, ((i - 1) * gePianoRoll::CELL_H) + 1, gePianoRoll::CELL_W, gePianoRoll::CELL_H,
		    (Fl_Align)(FL_ALIGN_LEFT | FL_ALIGN_CENTER));

		/* Print horizontal line */

		fl_color(G_COLOR_GREY_3);
		if (i < gePianoRoll::MAX_KEYS + 1)
			fl_line(0, i * gePianoRoll::CELL_H, gePianoRoll::CELL_W, +i * gePianoRoll::CELL_H);
	}

	fl_line_style(0);
	fl_end_offscreen();
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gePianoRollLegend::gePianoRollLegend()
: geScroll(0, 0, 0, 0)
{
	type(0);                      // no scrollbars
	add(new geContent(0, 0, 35)); // TODO - no magic number
}

/* -------------------------------------------------------------------------- */

void gePianoRollLegend::draw()
{
	geScroll::draw();

	const geompp::Rect<int> bounds = {x(), y(), w(), h()};

	drawLine(bounds.getTopLine(), G_COLOR_GREY_4);
	drawLine(bounds.getLeftLine(), G_COLOR_GREY_4);
	drawLine(bounds.getBottomLine().withShiftedY(-1), G_COLOR_GREY_4);
}
} // namespace giada::v
