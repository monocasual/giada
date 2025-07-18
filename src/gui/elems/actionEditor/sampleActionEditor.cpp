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

#include "src/gui/elems/actionEditor/sampleActionEditor.h"
#include "src/core/actions/action.h"
#include "src/glue/actionEditor.h"
#include "src/gui/dialogs/actionEditor/baseActionEditor.h"
#include "src/gui/dialogs/actionEditor/sampleActionEditor.h"
#include "src/gui/elems/actionEditor/sampleAction.h"
#include "src/gui/ui.h"
#include <FL/fl_draw.H>

extern giada::v::Ui* g_ui;

namespace giada::v
{
geSampleActionEditor::geSampleActionEditor(Pixel x, Pixel y, gdBaseActionEditor* b)
: geBaseActionEditor(x, y, 200, 40, b)
{
}

/* -------------------------------------------------------------------------- */

void geSampleActionEditor::rebuild(c::actionEditor::Data& d)
{
	m_data = &d;

	bool isSinglePressMode = m_data->sample->channelMode == SamplePlayerMode::SINGLE_PRESS;
	bool isAnyLoopMode     = m_data->sample->isLoopMode;

	/* Remove all existing actions and set a new width, according to the current
	zoom level. */

	clear();
	size(m_base->fullWidth, h());

	for (const m::Action& a1 : m_data->actions)
	{
		if (a1.event.getStatus() == m::MidiEvent::CHANNEL_CC || isNoteOffSinglePress(a1))
			continue;

		const m::Action& a2 = a1.nextId != 0 ? *c::actionEditor::findAction(a1.nextId) : m::Action{};

		Pixel px = x() + m_base->frameToPixel(a1.frame);
		Pixel py = y() + 4;
		Pixel pw = 0;
		Pixel ph = h() - 8;
		if (a2.isValid() && isSinglePressMode)
			pw = m_base->frameToPixel(a2.frame - a1.frame);

		geSampleAction* gsa = new geSampleAction(px, py, pw, ph, isSinglePressMode, a1, a2);
		add(gsa);
		resizable(gsa);
	}

	/* If channel is LOOP_ANY, deactivate it: a loop mode channel cannot hold
	keypress/keyrelease actions. */

	isAnyLoopMode ? deactivate() : activate();

	redraw();
}

/* -------------------------------------------------------------------------- */

void geSampleActionEditor::draw()
{
	/* Force height to match its parent's height. This widget belongs to a
	geScroll container (see geSplitScroll class in baseActionEditor.h) but
	there's nothing to scroll here actually. */

	size(w(), parent()->h());

	/* Draw basic boundaries (+ beat bars) and hide the unused area. Then draw
	children (the actions). */

	baseDraw();
	draw_children();
}

/* -------------------------------------------------------------------------- */

void geSampleActionEditor::onAddAction()
{
	Frame f = m_base->pixelToFrame(Fl::event_x() - x(), m_data->framesInBeat);
	c::actionEditor::recordSampleAction(m_data->channelId, static_cast<gdSampleActionEditor*>(m_base)->getActionType(), f);
}

/* -------------------------------------------------------------------------- */

void geSampleActionEditor::onDeleteAction()
{
	c::actionEditor::deleteSampleAction(m_data->channelId, m_action->a1);
}

/* -------------------------------------------------------------------------- */

void geSampleActionEditor::onMoveAction()
{
	Pixel ex = Fl::event_x() - m_action->pick;

	Pixel x1 = x();
	Pixel x2 = m_base->loopWidth + x() - m_action->w();

	if (ex < x1)
		ex = x1;
	else if (ex > x2)
		ex = x2;

	m_action->setPosition(ex);
}

/* -------------------------------------------------------------------------- */

void geSampleActionEditor::onResizeAction()
{
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

void geSampleActionEditor::onRefreshAction()
{
	namespace ca = c::actionEditor;

	Pixel p1   = m_action->x() - x();
	Pixel p2   = m_action->x() + m_action->w() - x();
	Frame f1   = 0;
	Frame f2   = 0;
	int   type = m_action->a1.event.getStatus();

	if (!m_action->isOnEdges())
	{
		f1 = m_base->pixelToFrame(p1, m_data->framesInBeat);
		f2 = m_base->pixelToFrame(p2, m_data->framesInBeat, /*snap=*/false) - (m_base->pixelToFrame(p1, m_data->framesInBeat, /*snap=*/false) - f1);
	}
	else if (m_action->onLeftEdge)
	{
		f1 = m_base->pixelToFrame(p1, m_data->framesInBeat);
		f2 = m_action->a2.frame;
	}
	else if (m_action->onRightEdge)
	{
		f1 = m_action->a1.frame;
		f2 = m_base->pixelToFrame(p2, m_data->framesInBeat);
	}

	ca::updateSampleAction(m_data->channelId, m_action->a1, type, f1, f2);
}

/* -------------------------------------------------------------------------- */

bool geSampleActionEditor::isNoteOffSinglePress(const m::Action& a)
{
	return m_data->sample->channelMode == SamplePlayerMode::SINGLE_PRESS &&
	       a.event.getStatus() == m::MidiEvent::CHANNEL_NOTE_OFF;
}
} // namespace giada::v