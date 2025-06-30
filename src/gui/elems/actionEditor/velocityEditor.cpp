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

#include "velocityEditor.h"
#include "envelopePoint.h"
#include "glue/actionEditor.h"
#include "gui/const.h"
#include "gui/dialogs/actionEditor/baseActionEditor.h"
#include "src/core/actions/action.h"
#include "utils/math.h"
#include <FL/Fl.H>
#include <FL/fl_draw.H>

namespace giada::v
{
geVelocityEditor::geVelocityEditor(Pixel x, Pixel y, gdBaseActionEditor* b)
: geBaseActionEditor(x, y, 200, 40, b)
{
}

/* -------------------------------------------------------------------------- */

void geVelocityEditor::draw()
{
	baseDraw();

	if (h() < geEnvelopePoint::SIDE)
		return;

	if (children() == 0)
		return;

	for (int i = 0; i < children(); i++)
	{
		geEnvelopePoint* p = static_cast<geEnvelopePoint*>(child(i));
		if (m_action == nullptr)
			p->position(p->x(), valueToY(p->a1.event.getVelocityFloat()));
		const Pixel x1 = p->x() + (geEnvelopePoint::SIDE / 2);
		const Pixel y1 = p->y();
		const Pixel y2 = y() + h();
		fl_color(p->hovered ? G_COLOR_LIGHT_2 : G_COLOR_LIGHT_1);
		fl_line(x1, y1, x1, y2);
	}

	draw_children();
}

/* -------------------------------------------------------------------------- */

Pixel geVelocityEditor::valueToY(float v) const
{
	return u::math::map(v, 0.0f, G_MAX_VELOCITY_FLOAT, y() + (h() - geEnvelopePoint::SIDE), y());
}

float geVelocityEditor::yToValue(Pixel px) const
{
	return u::math::map(px, h() - geEnvelopePoint::SIDE, 0, 0.0f, G_MAX_VELOCITY_FLOAT);
}

/* -------------------------------------------------------------------------- */

void geVelocityEditor::rebuild(c::actionEditor::Data& d)
{
	m_data = &d;

	/* Remove all existing actions and set a new width, according to the current
	zoom level. */

	clear();
	size(m_base->fullWidth, h());

	for (const m::Action& action : m_data->actions)
	{
		if (action.event.getStatus() != m::MidiEvent::CHANNEL_NOTE_ON)
			continue;

		const Pixel px = x() + m_base->frameToPixel(action.frame) - (geEnvelopePoint::SIDE / 2);
		const Pixel py = y() + valueToY(action.event.getVelocityFloat());

		add(new geEnvelopePoint(px, py, action));
	}

	resizable(nullptr);
	redraw();
}

/* -------------------------------------------------------------------------- */

void geVelocityEditor::onMoveAction()
{
	Pixel ey = Fl::event_y() - (geEnvelopePoint::SIDE / 2);

	const Pixel y1 = y();
	const Pixel y2 = y() + h() - geEnvelopePoint::SIDE;

	if (ey < y1)
		ey = y1;
	else if (ey > y2)
		ey = y2;

	m_action->position(m_action->x(), ey);
	redraw();
}

/* -------------------------------------------------------------------------- */

void geVelocityEditor::onRefreshAction()
{
	c::actionEditor::updateVelocity(m_action->a1, yToValue(m_action->y() - y()));

	m_base->rebuild(); // Rebuild pianoRoll as well
}
} // namespace giada::v