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


#include <cassert>
#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include "../../../utils/log.h"
#include "../../../utils/math.h"
#include "../../../core/const.h"
#include "../../../core/conf.h"
#include "../../../core/action.h"
#include "../../../core/clock.h"
#include "../../../core/midiChannel.h"
#include "../../../glue/actionEditor.h"
#include "../../dialogs/actionEditor/baseActionEditor.h"
#include "envelopePoint.h"
#include "velocityEditor.h"


using std::vector;


namespace giada {
namespace v
{
geVelocityEditor::geVelocityEditor(Pixel x, Pixel y, m::MidiChannel* ch)
:	geBaseActionEditor(x, y, 200, m::conf::velocityEditorH, ch)
{
}


/* -------------------------------------------------------------------------- */


geVelocityEditor::~geVelocityEditor()
{
	m::conf::velocityEditorH = h();
}


/* -------------------------------------------------------------------------- */


void geVelocityEditor::draw() 
{
	baseDraw();

	/* Print label. */

	fl_color(G_COLOR_GREY_4);
	fl_font(FL_HELVETICA, G_GUI_FONT_SIZE_BASE);
	fl_draw("Velocity", x()+4, y(), w(), h(), (Fl_Align) (FL_ALIGN_LEFT));

	if (children() == 0)
		return;

	Pixel side = geEnvelopePoint::SIDE / 2;

	for (int i=0; i<children(); i++) {
		geEnvelopePoint* p = static_cast<geEnvelopePoint*>(child(i));
		if (m_action == nullptr)
			p->position(p->x(), valueToY(p->a1->event.getVelocity()));
		Pixel x1 = p->x() + side;
		Pixel y1 = p->y();
		Pixel y2 = y() + h();
		fl_line(x1, y1, x1, y2);
	}

	draw_children();
}


/* -------------------------------------------------------------------------- */


Pixel geVelocityEditor::valueToY(int v) const
{
	/* Cast the input type of 'v' to float, to make the mapping more precise. */
	return u::math::map<float, Pixel>(v, 0, G_MAX_VELOCITY, y() + (h() - geEnvelopePoint::SIDE), y());
}


int geVelocityEditor::yToValue(Pixel px) const
{
	return u::math::map<Pixel, int>(px, h() - geEnvelopePoint::SIDE, 1, 0, G_MAX_VELOCITY);	
}


/* -------------------------------------------------------------------------- */


void geVelocityEditor::rebuild()
{
	namespace ca = c::actionEditor;

	/* Remove all existing actions and set a new width, according to the current
	zoom level. */

	clear();
	size(m_base->fullWidth, h());

	for (const m::Action* action : m_base->getActions())
	{
		if (action->event.getStatus() == m::MidiEvent::NOTE_OFF)
			continue;
		
		//gu_log("[geVelocityEditor::rebuild] f=%d\n", action->frame);

		Pixel px = x() + m_base->frameToPixel(action->frame);
		Pixel py = y() + valueToY(action->event.getVelocity());

		add(new geEnvelopePoint(px, py, action));
	}
	
	resizable(nullptr);
	redraw();
}


/* -------------------------------------------------------------------------- */


void geVelocityEditor::onMoveAction()    
{
	Pixel ey = Fl::event_y() - (geEnvelopePoint::SIDE / 2);

	Pixel y1 = y();
	Pixel y2 = y() + h() - geEnvelopePoint::SIDE;

	if (ey < y1) ey = y1; else if (ey > y2) ey = y2;

	m_action->position(m_action->x(), ey);
	redraw();
}


/* -------------------------------------------------------------------------- */


void geVelocityEditor::onRefreshAction() 
{
	c::actionEditor::updateVelocity(static_cast<m::MidiChannel*>(m_ch), m_action->a1, 
		yToValue(m_action->y() - y()));

	m_base->rebuild();  // Rebuild pianoRoll as well
}
}} // giada::v::