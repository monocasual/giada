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

#include "pianoRoll.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/midiEvent.h"
#include "glue/actionEditor.h"
#include "glue/channel.h"
#include "gui/dialogs/actionEditor/baseActionEditor.h"
#include "gui/elems/actionEditor/pianoItem.h"
#include "src/core/actions/action.h"
#include "utils/log.h"
#include "utils/math.h"
#include <FL/Fl.H>
#include <cassert>

namespace giada::v
{
gePianoRoll::gePianoRoll(Pixel X, Pixel Y, gdBaseActionEditor* b)
: geBaseActionEditor(X, Y, 200, CELL_H * MAX_KEYS, b)
{
	drawOffscreenGrid();
}

/* -------------------------------------------------------------------------- */

void gePianoRoll::drawOffscreenGrid()
{
	m_offscreenGrid = fl_create_offscreen(CELL_W, h());

	fl_begin_offscreen(m_offscreenGrid);
	fl_rectf(0, 0, CELL_W, h(), G_COLOR_GREY_1_5);
	fl_color(G_COLOR_GREY_3);
	fl_line_style(FL_DASH, 0, nullptr);

	for (int i = 1; i <= MAX_KEYS + 1; i++)
	{
		switch (i % KEYS)
		{
		case static_cast<int>(Notes::G):
		case static_cast<int>(Notes::E):
		case static_cast<int>(Notes::D):
		case static_cast<int>(Notes::B):
		case static_cast<int>(Notes::A):
			fl_rectf(0, i * CELL_H, CELL_W, CELL_H, G_COLOR_GREY_2);
			break;
		}
		if (i < MAX_KEYS + 1)
		{
			fl_color(G_COLOR_GREY_3);
			fl_line(0, i * CELL_H, CELL_W, i * CELL_H);
		}
	}

	fl_line_style(0);
	fl_end_offscreen();
}

/* -------------------------------------------------------------------------- */

void gePianoRoll::draw()
{
// TODO - is this APPLE thing still useful?
#if defined(__APPLE__)
	for (Pixel i = 0; i < m_base->fullWidth; i += 36) /// TODO: i < m_base->loopWidth is faster
		fl_copy_offscreen(x() + i, y(), CELL_W, h(), m_offscreenGrid, 1, 0);
#else
	for (Pixel i = 0; i < m_base->loopWidth; i += CELL_W)
		fl_copy_offscreen(x() + i, y(), CELL_W, h(), m_offscreenGrid, 0, 0);
#endif

	baseDraw(false);
	draw_children();
}

/* -------------------------------------------------------------------------- */

void gePianoRoll::onAddAction()
{
	Frame frame = m_base->pixelToFrame(Fl::event_x() - x(), m_data->framesInBeat);
	int   note  = yToNote(Fl::event_y() - y());
	c::actionEditor::recordMidiAction(m_data->channelId, note, G_MAX_VELOCITY_FLOAT,
	    frame);
}

/* -------------------------------------------------------------------------- */

void gePianoRoll::onDeleteAction()
{
	c::actionEditor::deleteMidiAction(m_data->channelId, m_action->a1);
}

/* -------------------------------------------------------------------------- */

void gePianoRoll::onMoveAction()
{
	/* Y computation:  - (CELL_H/2) is wrong: we should need the y pick value as 
	done with x. Let's change this when vertical piano zoom will be available. */

	Pixel ex = Fl::event_x() - m_action->pick;
	Pixel ey = snapToY(Fl::event_y() - y() - (CELL_H / 2)) + y();

	Pixel x1 = x();
	Pixel x2 = (m_base->loopWidth + x()) - m_action->w();
	Pixel y1 = y();
	Pixel y2 = y() + h();

	if (ex < x1)
		ex = x1;
	else if (ex > x2)
		ex = x2;
	if (ey < y1)
		ey = y1;
	else if (ey > y2)
		ey = y2;

	m_action->position(ex, ey);
}

/* -------------------------------------------------------------------------- */

void gePianoRoll::onResizeAction()
{
	if (!static_cast<gePianoItem*>(m_action)->isResizable())
		return;

	Pixel ex = Fl::event_x();

	Pixel x1 = x();
	Pixel x2 = m_base->loopWidth + x();

	if (ex < x1)
		ex = x1;
	else if (ex > x2)
		ex = x2;

	if (m_action->onRightEdge)
		m_action->setRightEdge(ex - m_action->x());
	else
		m_action->setLeftEdge(ex);
}

/* -------------------------------------------------------------------------- */

void gePianoRoll::onRefreshAction()
{
	namespace ca = c::actionEditor;

	Pixel p1 = m_action->x() - x();
	Pixel p2 = m_action->x() + m_action->w() - x();

	Frame f1 = 0;
	Frame f2 = 0;

	if (!m_action->isOnEdges())
	{
		f1 = m_base->pixelToFrame(p1, m_data->framesInBeat);
		f2 = m_base->pixelToFrame(p2, m_data->framesInBeat, /*snap=*/false) - (m_base->pixelToFrame(p1, m_data->framesInBeat, /*snap=*/false) - f1);
	}
	else if (m_action->onLeftEdge)
	{
		f1 = m_base->pixelToFrame(p1, m_data->framesInBeat);
		f2 = m_action->a2.frame;
		if (f1 == f2) // If snapping makes an action fall onto the other
			f1 -= G_DEFAULT_ACTION_SIZE;
	}
	else if (m_action->onRightEdge)
	{
		f1 = m_action->a1.frame;
		f2 = m_base->pixelToFrame(p2, m_data->framesInBeat);
		if (f1 == f2) // If snapping makes an action fall onto the other
			f2 += G_DEFAULT_ACTION_SIZE;
	}

	assert(f2 != 0);

	int   note     = yToNote(m_action->y() - y());
	float velocity = m_action->a1.event.getVelocityFloat();

	ca::updateMidiAction(m_data->channelId, m_action->a1, note, velocity, f1, f2);
}

/* -------------------------------------------------------------------------- */

int gePianoRoll::yToNote(Pixel p) const
{
	return gePianoRoll::MAX_KEYS - (p / gePianoRoll::CELL_H);
}

Pixel gePianoRoll::noteToY(int n) const
{
	return (MAX_KEYS * CELL_H) - (n * gePianoRoll::CELL_H);
}

Pixel gePianoRoll::snapToY(Pixel p) const
{
	return u::math::quantize(p, CELL_H);
}

Pixel gePianoRoll::getPianoItemW(Pixel px, const m::Action& a1, const m::Action& a2) const
{
	if (a2.isValid())
	{                            // Regular
		if (a1.frame > a2.frame) // Ring-loop
			return m_base->loopWidth - (px - x());
		return m_base->frameToPixel(a2.frame - a1.frame);
	}
	return geBaseAction::MIN_WIDTH; // Orphaned
}

/* -------------------------------------------------------------------------- */

void gePianoRoll::rebuild(c::actionEditor::Data& d)
{
	m_data = &d;

	/* Remove all existing actions and set a new width, according to the current
	zoom level. */

	clear();
	size(m_base->fullWidth, (MAX_KEYS + 1) * CELL_H);

	for (const m::Action& a1 : m_data->actions)
	{
		if (a1.event.getStatus() == m::MidiEvent::CHANNEL_NOTE_OFF)
			continue;

		assert(a1.isValid()); // a2 might be null if orphaned

		const m::Action& a2 = a1.nextId != 0 ? *c::actionEditor::findAction(a1.nextId) : m::Action{};

		Pixel px = x() + m_base->frameToPixel(a1.frame);
		Pixel py = y() + noteToY(a1.event.getNote());
		Pixel ph = CELL_H;
		Pixel pw = getPianoItemW(px, a1, a2);

		add(new gePianoItem(px, py, pw, ph, a1, a2));
	}

	redraw();
}
} // namespace giada::v
