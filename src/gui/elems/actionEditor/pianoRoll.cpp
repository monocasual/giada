/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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
#include "../../../core/conf.h"
#include "../../../core/const.h"
#include "../../../core/clock.h"
#include "../../../core/action.h"
#include "../../../core/midiEvent.h"
#include "../../../core/midiChannel.h"
#include "../../../utils/log.h"
#include "../../../utils/string.h"
#include "../../../utils/math.h"
#include "../../../glue/actionEditor.h"
#include "../../dialogs/actionEditor/baseActionEditor.h"
#include "pianoItem.h"
#include "noteEditor.h"
#include "pianoRoll.h"


using std::string;
using std::vector;


namespace giada {
namespace v
{
gePianoRoll::gePianoRoll(Pixel X, Pixel Y, Pixel W, m::MidiChannel* ch)
	: geBaseActionEditor(X, Y, W, 40, ch),
	  pick              (0)
{
	position(x(), m::conf::pianoRollY == -1 ? y()-(h()/2) : m::conf::pianoRollY);
}


/* -------------------------------------------------------------------------- */


void gePianoRoll::drawSurface1()
{
	surface1 = fl_create_offscreen(CELL_W, h());
	fl_begin_offscreen(surface1);

	/* Warning: only w() and h() come from this widget, x and y coordinates are 
	absolute, since we are writing in a memory chunk. */

	fl_rectf(0, 0, CELL_W, h(), G_COLOR_GREY_1);

	fl_line_style(FL_DASH, 0, nullptr);
	fl_font(FL_HELVETICA, G_GUI_FONT_SIZE_BASE);

	int octave = MAX_OCTAVES;

	for (int i=1; i<=MAX_KEYS+1; i++) {

		/* print key note label. C C# D D# E F F# G G# A A# B */

		string note = gu_iToString(octave);
		switch (i % KEYS) {
			case (int) Notes::G:
				fl_rectf(0, i*CELL_H, CELL_W, CELL_H, G_COLOR_GREY_2);
				note += " G"; 
				break;
			case (int) Notes::FS:
				note += " F#";
				break;
			case (int) Notes::F:
				note += " F";
				break;
			case (int) Notes::E:
				fl_rectf(0, i*CELL_H, CELL_W, CELL_H, G_COLOR_GREY_2);
				note += " E";
				break;
			case (int) Notes::DS:
				note += " D#";
				break;
			case (int) Notes::D:
				fl_rectf(0, i*CELL_H, CELL_W, CELL_H, G_COLOR_GREY_2);
				note += " D";
				break;
			case (int) Notes::CS:
				note += " C#";
				break;
			case (int) Notes::C:
				note += " C";
				octave--;
				break;
			case (int) Notes::B:
				fl_rectf(0, i*CELL_H, CELL_W, CELL_H, G_COLOR_GREY_2);
				note += " B";
				break;
			case (int) Notes::AS:
				note += " A#";
				break;
			case (int) Notes::A:
				fl_rectf(0, i*CELL_H, CELL_W, CELL_H, G_COLOR_GREY_2);
				note += " A";
				break;
			case (int) Notes::GS:
				note += " G#";
				break;
		}

		/* Print note name */

		fl_color(G_COLOR_GREY_3);
		fl_draw(note.c_str(), 4, ((i-1)*CELL_H)+1, CELL_W, CELL_H,
			(Fl_Align) (FL_ALIGN_LEFT | FL_ALIGN_CENTER));

		/* Print horizontal line */

		if (i < MAX_KEYS+1)
			fl_line(0, i*CELL_H, CELL_W, +i*CELL_H);
	}

	fl_line_style(0);
	fl_end_offscreen();
}


/* -------------------------------------------------------------------------- */


void gePianoRoll::drawSurface2()
{
	surface2 = fl_create_offscreen(CELL_W, h());

	fl_begin_offscreen(surface2);
	fl_rectf(0, 0, CELL_W, h(), G_COLOR_GREY_1);
	fl_color(G_COLOR_GREY_3);
	fl_line_style(FL_DASH, 0, nullptr);

	for (int i=1; i<=MAX_KEYS+1; i++) {
		switch (i % KEYS) {
			case (int) Notes::G:
			case (int) Notes::E:
			case (int) Notes::D:
			case (int) Notes::B:
			case (int) Notes::A:
				fl_rectf(0, i*CELL_H, CELL_W, CELL_H, G_COLOR_GREY_2);
				break;
		}
		if (i < MAX_KEYS+1) {
			fl_color(G_COLOR_GREY_3);
			fl_line(0, i*CELL_H, CELL_W, i*CELL_H);
		}
	}

	fl_line_style(0);
	fl_end_offscreen();
}


/* -------------------------------------------------------------------------- */


void gePianoRoll::draw()
{
	fl_copy_offscreen(x(), y(), CELL_W, h(), surface1, 0, 0);

#if defined(__APPLE__) // TODO - is this still useful?
	for (Pixel i=36; i<m_base->fullWidth; i+=36) /// TODO: i < ae->coverX is faster
		fl_copy_offscreen(x()+i, y(), CELL_W, h(), surface2, 1, 0);
#else
	for (Pixel i=CELL_W; i<m_base->loopWidth; i+=CELL_W)
		fl_copy_offscreen(x()+i, y(), CELL_W, h(), surface2, 0, 0);
#endif

	baseDraw(false);
	draw_children();
}


/* -------------------------------------------------------------------------- */


int gePianoRoll::handle(int e)
{
	if (e == FL_PUSH && Fl::event_button3()) {
		pick = Fl::event_y() - y();
		return geBaseActionEditor::handle(e);
	}
	if (e == FL_DRAG && Fl::event_button3()) {
		static_cast<geNoteEditor*>(parent())->scroll();
		return 1;
	}
	return geBaseActionEditor::handle(e);
}


/* -------------------------------------------------------------------------- */


void gePianoRoll::onAddAction()
{
	Frame frame = m_base->pixelToFrame(Fl::event_x() - x());
	int   note  = yToNote(Fl::event_y() - y());
	c::actionEditor::recordMidiAction(static_cast<m::MidiChannel*>(m_ch), note, G_MAX_VELOCITY, frame);
	
	m_base->rebuild();  // Rebuild velocityEditor as well
}


/* -------------------------------------------------------------------------- */


void gePianoRoll::onDeleteAction()
{
	c::actionEditor::deleteMidiAction(static_cast<m::MidiChannel*>(m_ch), m_action->a1);	
	
	m_base->rebuild();  // Rebuild velocityEditor as well
}


/* -------------------------------------------------------------------------- */


void gePianoRoll::onMoveAction()
{
	/* Y computation:  - (CELL_H/2) is wrong: we should need the y pick value as 
	done with x. Let's change this when vertical piano zoom will be available. */

	Pixel ex = Fl::event_x() - m_action->pick;
	Pixel ey = snapToY(Fl::event_y() - y() - (CELL_H/2)) + y();

	Pixel x1 = x();
	Pixel x2 = (m_base->loopWidth + x()) - m_action->w();
	Pixel y1 = y();
	Pixel y2 = y() + h();

	if (ex < x1) ex = x1; else if (ex > x2) ex = x2;
	if (ey < y1) ey = y1; else if (ey > y2) ey = y2;

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
	
	if (ex < x1) ex = x1; else if (ex > x2) ex = x2;

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

	if (!m_action->isOnEdges()) {
		f1 = m_base->pixelToFrame(p1);
		f2 = m_base->pixelToFrame(p2, /*snap=*/false) - (m_base->pixelToFrame(p1, /*snap=*/false) - f1);
	}	
	else if (m_action->onLeftEdge) {
		f1 = m_base->pixelToFrame(p1);
		f2 = m_action->a2->frame;
		if (f1 == f2) // If snapping makes an action fall onto the other
			f1 -= G_DEFAULT_ACTION_SIZE;
	}
	else if (m_action->onRightEdge) {
		f1 = m_action->a1->frame;
		f2 = m_base->pixelToFrame(p2);
		if (f1 == f2) // If snapping makes an action fall onto the other
			f2 += G_DEFAULT_ACTION_SIZE;
	}

	assert(f2 != 0);

	int note     = yToNote(m_action->y() - y());
	int velocity = m_action->a1->event.getVelocity();

	ca::updateMidiAction(static_cast<m::MidiChannel*>(m_ch), m_action->a1, note, 
		velocity, f1, f2);

	m_base->rebuild();  // Rebuild velocityEditor as well
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


Pixel gePianoRoll::getPianoItemW(Pixel px, const m::Action* a1, const m::Action* a2) const
{
	if (a2 != nullptr) {            // Regular
		if (a1->frame > a2->frame)  // Ring-loop
			return m_base->loopWidth - (px - x());
		return m_base->frameToPixel(a2->frame - a1->frame);
	}
	return geBaseAction::MIN_WIDTH;	// Orphaned
}


/* -------------------------------------------------------------------------- */


void gePianoRoll::rebuild()
{
	namespace ca = c::actionEditor;

	/* Remove all existing actions and set a new width, according to the current
	zoom level. */

	clear();
	size(m_base->fullWidth, (MAX_KEYS + 1) * CELL_H);

	for (const m::Action* action : m_base->getActions())
	{
		if (action->event.getStatus() == m::MidiEvent::NOTE_OFF)
			continue;

		const m::Action* a1 = action;
		const m::Action* a2 = action->next;

		assert(a1 != nullptr);  // a2 might be null if orphaned

		Pixel px = x() + m_base->frameToPixel(a1->frame);
		Pixel py = y() + noteToY(a1->event.getNote());
		Pixel ph = CELL_H;
		Pixel pw = getPianoItemW(px, a1, a2);

		add(new gePianoItem(px, py, pw, ph, a1, a2));
	}

	drawSurface1();
	drawSurface2();

	redraw();
}
}} // giada::v::