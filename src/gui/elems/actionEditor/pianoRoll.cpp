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


#include "../../../core/conf.h"
#include "../../../core/const.h"
#include "../../../core/mixer.h"
#include "../../../core/clock.h"
#include "../../../core/channel.h"
#include "../../../core/recorder.h"
#include "../../../core/kernelMidi.h"
#include "../../../utils/log.h"
#include "../../../utils/string.h"
#include "../../../glue/recorder.h"
#include "../../dialogs/gd_actionEditor.h"
#include "pianoItem.h"
#include "pianoItemOrphaned.h"
#include "noteEditor.h"
#include "pianoRoll.h"


using std::string;
using std::vector;
using namespace giada;


gePianoRoll::gePianoRoll(int X, int Y, int W, gdActionEditor* pParent)
	: geBaseActionEditor(X, Y, W, 40, pParent)
{

	resizable(nullptr);                   // don't resize children (i.e. pianoItem)
	size(W, (MAX_KEYS+1) * CELL_H);      // 128 MIDI channels * CELL_H height

	if (m::conf::pianoRollY == -1)
		position(x(), y()-(h()/2));  // center
	else
		position(x(), m::conf::pianoRollY);

	drawSurface1();
	drawSurface2();

	build();
}


/* -------------------------------------------------------------------------- */


void gePianoRoll::build()
{
	using namespace m::recorder;
	
	clear();

	int channel  = pParent->chan->index;
	int maxFrame = m::clock::getTotalFrames();

	vector<Composite> actions = c::recorder::getMidiActions(channel, maxFrame); 
	for (Composite composite : actions)
	{
		m::MidiEvent e1 = composite.a1.iValue;
		m::MidiEvent e2 = composite.a2.iValue;

		gu_log("[gePianoRoll] ((0x%X, 0x%X, f=%d) - (0x%X, 0x%X, f=%d))\n", 
			e1.getStatus(), e1.getNote(), composite.a1.frame,
			e2.getStatus(), e2.getNote(), composite.a2.frame
		);

		if (composite.a2.frame != -1)
			add(new gePianoItem(0, 0, x(), y(), composite.a1, composite.a2, pParent));
		else
			add(new gePianoItemOrphaned(0, 0, x(), y(), composite.a1, pParent));
	}

	redraw();
}


/* -------------------------------------------------------------------------- */


void gePianoRoll::drawSurface1()
{
	surface1 = fl_create_offscreen(CELL_W, h());
	fl_begin_offscreen(surface1);

	/* warning: only w() and h() come from this widget, x and y coordinates
	 * are absolute, since we are writing in a memory chunk */

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

#if defined(__APPLE__)
	for (int i=36; i<pParent->totalWidth; i+=36) /// TODO: i < pParent->coverX is faster
		fl_copy_offscreen(x()+i, y(), CELL_W, h(), surface2, 1, 0);
#else
	for (int i=CELL_W; i<pParent->totalWidth; i+=CELL_W) /// TODO: i < pParent->coverX is faster
		fl_copy_offscreen(x()+i, y(), CELL_W, h(), surface2, 0, 0);
#endif

	baseDraw(false);
	draw_children();
}


/* -------------------------------------------------------------------------- */


int gePianoRoll::handle(int e)
{
	int ret = Fl_Group::handle(e);
	switch (e) {
		case FL_PUSH:	{

			/* avoid click on grey area */

			if (Fl::event_x() >= pParent->coverX) {
				ret = 1;
				break;
			}

			push_y = Fl::event_y() - y();

			if (Fl::event_button1()) {

				/* ax is driven by grid, ay by the height in px of each note. */

				int ax = Fl::event_x();
				int ay = Fl::event_y();

				/* Vertical snap. */

				int edge = (ay-y()) % CELL_H;
				if (edge != 0) ay -= edge;

				/* If there are no pianoItems below the mouse, add a new one. */

				gePianoItem* pianoItem = dynamic_cast<gePianoItem*>(Fl::belowmouse());
				if (pianoItem == nullptr)
					recordAction(yToNote(ay-y()), (ax-x()) * pParent->zoom);
			}
			ret = 1;
			break;
		}

		case FL_DRAG:	{

			if (Fl::event_button3()) {

				geNoteEditor* prc = static_cast<geNoteEditor*>(parent());
				position(x(), Fl::event_y() - push_y);

				if (y() > prc->y())
					position(x(), prc->y());
				else
				if (y() < prc->y()+prc->h()-h())
					position(x(), prc->y()+prc->h()-h());

				prc->redraw();
			}
			ret = 1;
			break;
		}

		case FL_MOUSEWHEEL: {   // nothing to do, just avoid small internal scroll
			ret = 1;
			break;
		}
	}
	return ret;
}


/* -------------------------------------------------------------------------- */


void gePianoRoll::recordAction(int note, int frame_a, int frame_b)
{
	c::recorder::recordMidiAction(pParent->chan->index, note, frame_a, frame_b);
	pParent->chan->hasActions = true;
	build();
}


/* -------------------------------------------------------------------------- */


int gePianoRoll::yToNote(int y)
{
	return gePianoRoll::MAX_KEYS - (y / gePianoRoll::CELL_H);
}

/* -------------------------------------------------------------------------- */


void gePianoRoll::updateActions()
{
	for (int k=0; k<children(); k++)
		static_cast<geBasePianoItem*>(child(k))->reposition(x());
}
