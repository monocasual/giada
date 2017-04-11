/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#include "../../../core/kernelMidi.h"
#include "../../../core/const.h"
#include "../../../core/mixer.h"
#include "../../../core/channel.h"
#include "../../../core/clock.h"
#include "../../../core/midiChannel.h"
#include "../../dialogs/gd_actionEditor.h"
#include "noteEditor.h"
#include "pianoRoll.h"
#include "gridTool.h"
#include "pianoItem.h"


using namespace giada;


gePianoItem::gePianoItem(int X, int Y, int rel_x, int rel_y, recorder::action *_a,
  recorder::action *_b, gdActionEditor *pParent)
	: Fl_Box  (X, Y, MIN_WIDTH, gePianoRoll::CELL_H),
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
		note    = kernelMidi::getB2(a->iValue);
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

	geNoteEditor *pPiano = static_cast<geNoteEditor*>(parent());

	for (int i=0; i<pPiano->children(); i++) {

		gePianoItem *pItem = static_cast<gePianoItem*>(pPiano->child(i));

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
	int _w = w() > MIN_WIDTH ? w() : MIN_WIDTH;
	fl_rectf(x(), y()+2, _w, h()-3, (Fl_Color) selected ? COLOR_BD_1 : COLOR_BG_2);
}


/* -------------------------------------------------------------------------- */


void gePianoItem::record()
{
	/* avoid frame overflow */

	int overflow = frame_b - clock::getTotalFrames();
	if (overflow > 0) {
		frame_b -= overflow;
		frame_a -= overflow;
	}

	event_a |= (MIDI_NOTE_ON);
	event_a |= (note << 16);   // note value
	event_a |= (MIDI_VELOCITY);
	event_a |= (0x00);

	event_b |= (MIDI_NOTE_OFF);
	event_b |= (note << 16);   // note value
	event_b |= (MIDI_VELOCITY);
	event_b |= (0x00);

	recorder::rec(pParent->chan->index, G_ACTION_MIDI, frame_a, event_a);
	recorder::rec(pParent->chan->index, G_ACTION_MIDI, frame_b, event_b);
  pParent->chan->hasActions = true;
}


/* -------------------------------------------------------------------------- */


void gePianoItem::remove()
{
	recorder::deleteAction(pParent->chan->index, frame_a, G_ACTION_MIDI, true,
    &mixer::mutex_recs, event_a, 0.0);
	recorder::deleteAction(pParent->chan->index, frame_b, G_ACTION_MIDI, true,
    &mixer::mutex_recs, event_b, 0.0);

	/* send a note-off in case we are deleting it in a middle of a key_on
	 * key_off sequence. */

	static_cast<MidiChannel*>(pParent->chan)->sendMidi(event_b);

  static_cast<gePianoRoll*>(parent())->cursorOnItem = false;
}


/* -------------------------------------------------------------------------- */


int gePianoItem::handle(int e)
{
	int ret = 0;

	switch (e) {

		case FL_ENTER: {
      static_cast<gePianoRoll*>(parent())->cursorOnItem = true;
			selected = true;
			ret = 1;
			redraw();
			break;
		}

		case FL_LEAVE: {
			fl_cursor(FL_CURSOR_DEFAULT, FL_WHITE, FL_BLACK);
      static_cast<gePianoRoll*>(parent())->cursorOnItem = false;
			selected = false;
			ret = 1;
			redraw();
			break;
		}

		case FL_MOVE: {
			onLeftEdge  = false;
			onRightEdge = false;

			if (Fl::event_x() >= x() && Fl::event_x() < x()+HANDLE_WIDTH) {
				onLeftEdge = true;
				fl_cursor(FL_CURSOR_WE, FL_WHITE, FL_BLACK);
			}
			else
			if (Fl::event_x() >= x()+w()-HANDLE_WIDTH && Fl::event_x() <= x()+w()) {
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
				static_cast<geNoteEditor*>(parent())->redraw();
			}
			ret = 1;
			break;
		}

		case FL_DRAG: {

			changed = true;

			geNoteEditor *pr = static_cast<geNoteEditor*>(parent());
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
				if (nx > x()+w()-MIN_WIDTH) {
					nx = x()+w()-MIN_WIDTH;
					nw = MIN_WIDTH;
				}
				resize(nx, ny, nw, h());
			}
			else
			if (onRightEdge) {
				nw = Fl::event_x()-x();
				if (Fl::event_x() < x()+MIN_WIDTH)
					nw = MIN_WIDTH;
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
			static_cast<geNoteEditor*>(parent())->redraw();
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

			static_cast<geNoteEditor*>(parent())->redraw();

			ret = 1;
			break;
		}
	}
	return ret;
}


/* -------------------------------------------------------------------------- */


int gePianoItem::getNote(int rel_y)
{
  return gePianoRoll::MAX_KEYS - (rel_y / gePianoRoll::CELL_H);
}


/* -------------------------------------------------------------------------- */


int gePianoItem::getRelY()
{
  return y() - parent()->y();
}


/* -------------------------------------------------------------------------- */


int gePianoItem::getRelX()
{
  return x() - parent()->x();
}


/* -------------------------------------------------------------------------- */


int gePianoItem::getY(int note)
{
  return (gePianoRoll::MAX_KEYS * gePianoRoll::CELL_H) - (note * gePianoRoll::CELL_H);
}
