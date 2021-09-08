/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * beatMeter
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

#include "sequencer.h"
#include "core/const.h"
#include "gui/drawing.h"
#include "utils/math.h"
#include <FL/fl_draw.H>

namespace giada::v
{
geSequencer::geSequencer(int x, int y, int w, int h)
: Fl_Box(x-90, y-10, w-100, h)
{
	copy_tooltip("Main sequencer");
}

/* -------------------------------------------------------------------------- */

void geSequencer::refresh()
{
	m_data = c::main::getSequencer();
	redraw();
}

/* -------------------------------------------------------------------------- */

void geSequencer::draw()
{
	m_background = geompp::Rect(x(), y(), w(), h());
	m_cell       = geompp::Rect(x(), y(), w() / G_MAX_BEATS, h()).reduced(0, REC_BARS_H);

	/* Cleanup */
	drawRectf(m_background, FL_BACKGROUND_COLOR);

	if (m_data.isFreeModeInputRec)
		drawRecBars();

	drawBody();
	drawCursor();
}

/* -------------------------------------------------------------------------- */

void geSequencer::drawBody() const
{
	const geompp::Rect body = m_background.reduced(0, REC_BARS_H);
	const geompp::Line line = m_cell.getHeightAsLine();

	/* Background and borders. */

	drawRectf(body, FL_BACKGROUND_COLOR);
	drawRect(body, G_COLOR_GREY_4);

	/* Beat lines. */

	for (int i = 1; i <= m_data.beats; i++)
		drawLine(line.withShiftedX(m_cell.w * i), G_COLOR_GREY_4);

	/* Bar lines. */

	const int delta = m_data.beats / m_data.bars;
	for (int i = 1; i < m_data.bars; i++)
		drawLine(line.withShiftedX(m_cell.w * i * delta), G_COLOR_LIGHT_1);

	/* Unused grey area. */

	drawRectf(body.withTrimmedLeft(m_data.beats * m_cell.w), G_COLOR_GREY_4);
}

/* -------------------------------------------------------------------------- */

void geSequencer::drawRecBars() const
{
	int length = u::math::map(m_data.recPosition, m_data.recMaxLength, w());

	drawRectf(geompp::Rect(x(), y(), length, h()), G_COLOR_LIGHT_1);
}

/* -------------------------------------------------------------------------- */

void geSequencer::drawCursor(int beat, Fl_Color color) const
{
	// TODO withW(...): FLTK glitch?
	drawRectf(m_cell.withShiftedX(beat * m_cell.w).reduced(CURSOR_PAD).withW(m_cell.w - CURSOR_PAD - 2), color);
}

/* -------------------------------------------------------------------------- */

void geSequencer::drawCursor() const
{
	Fl_Color color = m_data.shouldBlink ? FL_BACKGROUND_COLOR : G_COLOR_LIGHT_1;

	if (m_data.isFreeModeInputRec)
	{
		for (int i = 0; i < m_data.beats; i++)
			drawCursor(i, color);
	}
	else
		drawCursor(m_data.currentBeat, color);
}
} // namespace giada::v
