/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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

	bool isAnyLoopMode = m_data->sample->isLoopMode;

	/* Remove all existing actions and set a new width, according to the current
	zoom level. */

	clear();
	size(m_base->fullWidth, h());

	for (const m::Action* a1 : m_data->actions)
	{
		if (a1->event.getStatus() == m::MidiEvent::CHANNEL_CC)
			continue;

		Pixel px = x() + m_base->tickToPixel(a1->tick);
		Pixel py = y() + 4;
		Pixel pw = 0;
		Pixel ph = h() - 8;

		geSampleAction* gsa = new geSampleAction(px, py, pw, ph, *a1);
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
	Tick t = m_base->pixelToTickSnapped(Fl::event_x() - x());
	c::actionEditor::recordSampleAction(m_data->channelId,
	    static_cast<gdSampleActionEditor*>(m_base)->getActionType(), t);
}

/* -------------------------------------------------------------------------- */

void geSampleActionEditor::onDeleteAction()
{
	c::actionEditor::deleteSampleAction(m_action->a1->id);
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
	const Pixel leftEdge  = x();
	const Pixel rightEdge = m_base->loopWidth + x();
	Pixel       ex        = std::clamp(Fl::event_x(), leftEdge, rightEdge);

	if (m_action->onRightEdge)
		m_action->setRightEdge(ex - m_action->x());
	else
		m_action->setLeftEdge(ex);
}

/* -------------------------------------------------------------------------- */

void geSampleActionEditor::onRefreshAction()
{
	const Pixel p    = m_action->x() - x();
	const Tick  t    = m_base->pixelToTickSnapped(p);
	const int   type = m_action->a1->event.getStatus();

	c::actionEditor::updateSampleAction(m_data->channelId, m_action->a1->id, type, t);
}
} // namespace giada::v