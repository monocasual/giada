/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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


#include "../../core/kernelMidi.h"
#include "../../core/mixer.h"
#include "../../core/channel.h"
#include "../../core/midiChannel.h"
#include "../dialogs/gd_actionEditor.h"
#include "noteEditor.h"
#include "pianoItem.h"


extern KernelMidi    G_KernelMidi;
extern Mixer         G_Mixer;
extern Recorder			 G_Recorder;



gePianoItem::gePianoItem(int X, int Y, int rel_x, int rel_y, Recorder::action *_a,
  Recorder::action *_b, gdActionEditor *pParent)
	: Fl_Box  (X, Y, 20, gePianoRoll::CELL_H-5),
	  a       (_a),
	  b       (_b),
		pParent (pParent),
		selected(false),
		event_a (0x00),
		event_b (0x00),
		changed (false)
{
	/* a is a pointer: action exists, needs to be displayed */

	if (a) {
		note    = G_KernelMidi.getB2(a->iValue);
		frame_a = a->frame;
		frame_b = b->frame;
		event_a = a->iValue;
		event_b = b->iValue;
		int newX = rel_x + (frame_a / pParent->zoom);
		int newY = rel_y + getY(note);
		int newW = (frame_b - frame_a) / pParent->zoom;
		resize(newX, newY, newW, h());
	}

	/* a is null: action needs to be recorded from scratch */

	else {
		note    = getNote(rel_y);
		frame_a = rel_x * pParent->zoom;
		frame_b = (rel_x + 20) * pParent->zoom;
		record();
		size((frame_b - frame_a) / pParent->zoom, h());
	}
}


/* -------------------------------------------------------------------------- */


bool gePianoItem::overlap()
{
	/* when 2 segments overlap?
	 * start = the highest value between the two starting points
	 * end   = the lowest value between the two ending points
	 * if start < end then there's an overlap of end-start pixels. */

	geNoteEditor *pPiano = (geNoteEditor*) parent();

	for (int i=0; i<pPiano->children(); i++) {

		gePianoItem *pItem = (gePianoItem*) pPiano->child(i);

		/* don't check against itself and with different y positions */

		if (pItem == this || pItem->y() != y())
			continue;

		int start = pItem->x() >= x() ? pItem->x() : x();
		int end   = pItem->x()+pItem->w() < x()+w() ? pItem->x()+pItem->w() : x()+w();
		if (start < end)
			return true;
	}

	return false;
}


/* -------------------------------------------------------------------------- */


void gePianoItem::draw()
{
	int _w = w() > 4 ? w() : 4;
	//gu_log("[gePianoItem] draw me (%p) at x=%d\n", (void*)this, x());
	fl_rectf(x(), y(), _w, h(), (Fl_Color) selected ? COLOR_BD_1 : COLOR_BG_2);
}


/* -------------------------------------------------------------------------- */


void gePianoItem::record()
{
	/* avoid frame overflow */

	int overflow = frame_b - G_Mixer.totalFrames;
	if (overflow > 0) {
		frame_b -= overflow;
		frame_a -= overflow;
	}

	/* note off */
	/** FIXME - use constants */
	event_a |= (0x90 << 24);   // note on
	event_a |= (note << 16);   // note value
	event_a |= (0x3F <<  8);   // velocity
	event_a |= (0x00);

	event_b |= (0x80 << 24);   // note off
	event_b |= (note << 16);   // note value
	event_b |= (0x3F <<  8);   // velocity
	event_b |= (0x00);

	G_Recorder.rec(pParent->chan->index, ACTION_MIDI, frame_a, event_a);
	G_Recorder.rec(pParent->chan->index, ACTION_MIDI, frame_b, event_b);
}


/* -------------------------------------------------------------------------- */


void gePianoItem::remove()
{
	G_Recorder.deleteAction(pParent->chan->index, frame_a, ACTION_MIDI, true, event_a, 0.0);
	G_Recorder.deleteAction(pParent->chan->index, frame_b, ACTION_MIDI, true, event_b, 0.0);

	/* send a note-off in case we are deleting it in a middle of a key_on
	 * key_off sequence. */

	((MidiChannel*) pParent->chan)->sendMidi(event_b);
}


/* -------------------------------------------------------------------------- */


int gePianoItem::handle(int e)
{
	int ret = 0;

	switch (e) {

		case FL_ENTER: {
			selected = true;
			ret = 1;
			redraw();
			break;
		}

		case FL_LEAVE: {
			fl_cursor(FL_CURSOR_DEFAULT, FL_WHITE, FL_BLACK);
			selected = false;
			ret = 1;
			redraw();
			break;
		}

		case FL_MOVE: {
			onLeftEdge  = false;
			onRightEdge = false;

			if (Fl::event_x() >= x() && Fl::event_x() < x()+4) {
				onLeftEdge = true;
				fl_cursor(FL_CURSOR_WE, FL_WHITE, FL_BLACK);
			}
			else
			if (Fl::event_x() >= x()+w()-4 && Fl::event_x() <= x()+w()) {
				onRightEdge = true;
				fl_cursor(FL_CURSOR_WE, FL_WHITE, FL_BLACK);
			}
			else
				fl_cursor(FL_CURSOR_DEFAULT, FL_WHITE, FL_BLACK);

			ret = 1;
			break;
		}

		case FL_PUSH: {

			push_x = Fl::event_x() - x();
			old_x  = x();
			old_w  = w();

			if (Fl::event_button3()) {
				fl_cursor(FL_CURSOR_DEFAULT, FL_WHITE, FL_BLACK);
				remove();
				hide();   // for Windows
				Fl::delete_widget(this);
				((geNoteEditor*)parent())->redraw();
			}
			ret = 1;
			break;
		}

		case FL_DRAG: {

			changed = true;

			geNoteEditor *pr = (geNoteEditor*) parent();
			int coverX     = pParent->coverX + pr->x(); // relative coverX
			int nx, ny, nw;

			if (onLeftEdge) {
				nx = Fl::event_x();
				ny = y();
				nw = x()-Fl::event_x()+w();
				if (nx < pr->x()) {
					nx = pr->x();
					nw = w()+x()-pr->x();
				}
				else
				if (nx > x()+w()-8) {
					nx = x()+w()-8;
					nw = 8;
				}
				resize(nx, ny, nw, h());
			}
			else
			if (onRightEdge) {
				nw = Fl::event_x()-x();
				if (Fl::event_x() < x()+8)
					nw = 8;
				else
				if (Fl::event_x() > coverX)
					nw = coverX-x();
				size(nw, h());
			}
			else {
				nx = Fl::event_x() - push_x;
				if (nx < pr->x()+1)
					nx = pr->x()+1;
				else
				if (nx+w() > coverX)
					nx = coverX-w();

				/* snapping */

				if (pParent->gridTool->isOn())
					nx = pParent->gridTool->getSnapPoint(nx-pr->x()) + pr->x() - 1;

				position(nx, y());
			}

			/* update screen */

			redraw();
			((geNoteEditor*)parent())->redraw();
			ret = 1;
			break;
		}

		case FL_RELEASE: {

			/* delete & record the action, only if it doesn't overlap with
			 * another one */

			if (overlap()) {
				resize(old_x, y(), old_w, h());
				redraw();
			}
			else
			if (changed) {
				remove();
				note    = getNote(getRelY());
				frame_a = getRelX() * pParent->zoom;
				frame_b = (getRelX()+w()) * pParent->zoom;
				record();
				changed = false;
			}

			((geNoteEditor*)parent())->redraw();

			ret = 1;
			break;
		}
	}
	return ret;
}


/* -------------------------------------------------------------------------- */


int gePianoItem::getNote(int rel_y)
{
  return gePianoRoll::MAX_NOTES - (rel_y / gePianoRoll::CELL_H);
}


/* -------------------------------------------------------------------------- */


int gePianoItem::getRelY()
{
  return y() - parent()->y() - 3;
}


/* -------------------------------------------------------------------------- */


int gePianoItem::getRelX()
{
  return x() - parent()->x();
}


/* -------------------------------------------------------------------------- */


int gePianoItem::getY(int note)
{
  return (gePianoRoll::MAX_NOTES * gePianoRoll::CELL_H) - (note * gePianoRoll::CELL_H);
}
