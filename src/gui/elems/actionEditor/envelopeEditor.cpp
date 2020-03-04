/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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
#include "utils/log.h"
#include "utils/math.h"
#include "core/const.h"
#include "core/conf.h"
#include "core/action.h"
#include "core/recorder.h"
#include "glue/actionEditor.h"
#include "glue/channel.h"
#include "gui/dialogs/actionEditor/baseActionEditor.h"
#include "envelopePoint.h"
#include "envelopeEditor.h"


namespace giada {
namespace v
{
geEnvelopeEditor::geEnvelopeEditor(Pixel x, Pixel y, const char* l, gdBaseActionEditor* b)
: geBaseActionEditor(x, y, 200, m::conf::conf.envelopeEditorH, b)
{
	copy_label(l);
}


/* -------------------------------------------------------------------------- */


geEnvelopeEditor::~geEnvelopeEditor()
{
	m::conf::conf.envelopeEditorH = h();
}


/* -------------------------------------------------------------------------- */


void geEnvelopeEditor::draw() 
{
	baseDraw();

	/* Print label. */

	fl_color(G_COLOR_GREY_4);
	fl_font(FL_HELVETICA, G_GUI_FONT_SIZE_BASE);
	fl_draw(label(), x()+4, y(), w(), h(), (Fl_Align) (FL_ALIGN_LEFT));

	if (children() == 0)
		return;

	Pixel side = geEnvelopePoint::SIDE / 2;

	Pixel x1 = child(0)->x() + side;
	Pixel y1 = child(0)->y() + side;
	Pixel x2 = 0;
	Pixel y2 = 0;

	/* For each point: 
		- paint the connecting line with the next one;
		- reposition it on the y axis, only if there's no point selected (dragged
	    around). */

	for (int i=0; i<children(); i++) {
		geEnvelopePoint* p = static_cast<geEnvelopePoint*>(child(i));
		if (m_action == nullptr)
			p->position(p->x(), valueToY(p->a1.event.getVelocity()));
		if (i > 0) {
			x2 = p->x() + side;
			y2 = p->y() + side;
			fl_line(x1, y1, x2, y2);
			x1 = x2;
			y1 = y2;
		}
	}

	draw_children();
}


/* -------------------------------------------------------------------------- */


void geEnvelopeEditor::rebuild(c::actionEditor::Data& d)
{
	m_data = &d;

	/* Remove all existing actions and set a new width, according to the current
	zoom level. */

	clear();
	size(m_base->fullWidth, h());

	for (const m::Action& a : m_data->actions) {
		if (a.event.getStatus() != m::MidiEvent::ENVELOPE)
			continue;
		add(new geEnvelopePoint(frameToX(a.frame), valueToY(a.event.getVelocity()), a)); 		
	}

	resizable(nullptr);

	redraw();
}


/* -------------------------------------------------------------------------- */


bool geEnvelopeEditor::isFirstPoint() const
{
	return find(m_action) == 0;
}


bool geEnvelopeEditor::isLastPoint() const
{
	return find(m_action) == children() - 1;
}


/* -------------------------------------------------------------------------- */


Pixel geEnvelopeEditor::frameToX(Frame frame) const
{
	return x() + m_base->frameToPixel(frame) - (geEnvelopePoint::SIDE / 2);
}


Pixel geEnvelopeEditor::valueToY(int value) const
{
	return u::math::map<int, Pixel>(value, 0, G_MAX_VELOCITY, y() + (h() - geEnvelopePoint::SIDE), y());
}


int geEnvelopeEditor::yToValue(Pixel pixel, Pixel offset) const
{
	return u::math::map<Pixel, int>(pixel, h() - offset, 0, 0, G_MAX_VELOCITY);	
}


/* -------------------------------------------------------------------------- */


void geEnvelopeEditor::onAddAction()     
{
	Frame f = m_base->pixelToFrame(Fl::event_x() - x());
	int   v = yToValue(Fl::event_y() - y());
	
	c::actionEditor::recordEnvelopeAction(m_data->channelId, f, v);
	
	m_base->rebuild(); // TODO - USELESS
}


/* -------------------------------------------------------------------------- */


void geEnvelopeEditor::onDeleteAction()  
{
	c::actionEditor::deleteEnvelopeAction(m_data->channelId, m_action->a1);
		
	m_base->rebuild(); // TODO - USELESS
}


/* -------------------------------------------------------------------------- */


void geEnvelopeEditor::onMoveAction()    
{
	Pixel side = geEnvelopePoint::SIDE / 2;
	Pixel ex   = Fl::event_x() - side;
	Pixel ey   = Fl::event_y() - side;

	Pixel x1 = x() - side;
	Pixel x2 = m_base->loopWidth + x() - side;
	Pixel y1 = y();
	Pixel y2 = y() + h() - geEnvelopePoint::SIDE;

	/* x-axis constraints. */
	if      (isFirstPoint() || ex < x1) ex = x1; 
	else if (isLastPoint()  || ex > x2) ex = x2;

	/* y-axis constraints. */
	if (ey < y1) ey = y1; else if (ey > y2) ey = y2;

	m_action->position(ex, ey);
	redraw();
}


/* -------------------------------------------------------------------------- */


void geEnvelopeEditor::onRefreshAction() 
{
	Frame f = m_base->pixelToFrame((m_action->x() - x()) + geEnvelopePoint::SIDE / 2);
	float v = yToValue(m_action->y() - y(), geEnvelopePoint::SIDE);
	c::actionEditor::updateEnvelopeAction(m_data->channelId, m_action->a1, f, v);

	m_base->rebuild();
}
}} // giada::v::
