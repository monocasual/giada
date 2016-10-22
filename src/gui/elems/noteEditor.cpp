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


#include <FL/fl_draw.H>
#include "../../core/channel.h"
#include "../../core/midiChannel.h"
#include "../../core/const.h"
#include "../../core/mixer.h"
#include "../../core/kernelMidi.h"
#include "../../core/conf.h"
#include "../../utils/log.h"
#include "../dialogs/gd_mainWindow.h"
#include "../dialogs/gd_actionEditor.h"
#include "pianoItem.h"
#include "noteEditor.h"


extern gdMainWindow *mainWin;
extern Mixer         G_Mixer;
extern Recorder			 G_Recorder;
extern KernelMidi    G_KernelMidi;
extern Conf	         G_Conf;


geNoteEditor::geNoteEditor(int x, int y, gdActionEditor *pParent)
  : Fl_Scroll(x, y, 200, 422),
    pParent  (pParent)
{
	size(pParent->totalWidth, G_Conf.pianoRollH);
	pianoRoll = new gePianoRoll(x, y, pParent->totalWidth, pParent);
}


/* -------------------------------------------------------------------------- */


geNoteEditor::~geNoteEditor()
{
	clear();
	G_Conf.pianoRollH = h();
	G_Conf.pianoRollY = pianoRoll->y();
}


/* -------------------------------------------------------------------------- */


void geNoteEditor::updateActions()
{
	pianoRoll->updateActions();
}


/* -------------------------------------------------------------------------- */


void geNoteEditor::draw()
{
	pianoRoll->size(this->w(), pianoRoll->h());  /// <--- not optimal

	/* clear background */

	fl_rectf(x(), y(), w(), h(), COLOR_BG_MAIN);

	/* clip pianoRoll to pianoRollContainer size */

	fl_push_clip(x(), y(), w(), h());
	draw_child(*pianoRoll);
	fl_pop_clip();

	fl_color(COLOR_BD_0);
	fl_line_style(0);
	fl_rect(x(), y(), pParent->totalWidth, h());
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


gePianoRoll::gePianoRoll(int X, int Y, int W, class gdActionEditor *pParent)
 : geBaseActionEditor(X, Y, W, 40, pParent)
{
	resizable(nullptr);                      // don't resize children (i.e. pianoItem)
	size(W, (MAX_NOTES+1) * CELL_H);      // 128 MIDI channels * 15 px height

	if (G_Conf.pianoRollY == -1)
		position(x(), y()-(h()/2));  // center
	else
		position(x(), G_Conf.pianoRollY);

	drawSurface1();
	drawSurface2();

	/* add actions when the window is opened. Position is zoom-based. MIDI
	 * actions come always in pair: start + end. */

	G_Recorder.sortActions();

	Recorder::action *a2   = nullptr;
	Recorder::action *prev = nullptr;

	for (unsigned i=0; i<G_Recorder.frames.size(); i++) {
		for (unsigned j=0; j<G_Recorder.global.at(i).size(); j++) {

			/* don't show actions > than the grey area */
			/** FIXME - can we move this to the outer cycle? */

			if (G_Recorder.frames.at(i) > G_Mixer.totalFrames)
				continue;

			Recorder::action *a1 = G_Recorder.global.at(i).at(j);

      /* Skip action if:
        - does not belong to this channel
        - is not a MIDI action (we only want MIDI things here)
        - is the previous one (we have already checked it)
        - (later on) if it's NOTE_OFF (0x80): we want note on only */

      if (a1->chan != pParent->chan->index)
				continue;
      if (a1->type != ACTION_MIDI)
        continue;
			if (a1 == prev)
				continue;

			/* extract MIDI infos from a1: if is note off skip it, we are looking
			 * for note on only */

			int a1_type = G_KernelMidi.getB1(a1->iValue);
			int a1_note = G_KernelMidi.getB2(a1->iValue);

			if (a1_type == 0x80) // NOTE_OFF
				continue;

			/* Search for the next action. Must have: same channel, ACTION_MIDI,
      greater than a1->frame and with MIDI properties of note_off (0x80), same
      note of a1, any velocity value (0xFF) because we just don't care about the
      velocity of a note_off. */

			G_Recorder.getNextAction(a1->chan, ACTION_MIDI, a1->frame, &a2,
					G_KernelMidi.getIValue(0x80, a1_note, 0xFF));

			/* next action note_off found: add a new gePianoItem to piano roll */

			if (a2) {
				new gePianoItem(0, 0, x(), y()+3, a1, a2, pParent);
				prev = a2;
				a2 = nullptr;
			}
			else
				gu_log("[geNoteEditor] recorder didn't find requested action!\n");
        // TODO - create new gOrphanedPianoItem
	  }
	}

	end();
}


/* -------------------------------------------------------------------------- */


void gePianoRoll::drawSurface1()
{
	surface1 = fl_create_offscreen(40, h());
	fl_begin_offscreen(surface1);

	/* warning: only w() and h() come from this widget, x and y coordinates
	 * are absolute, since we are writing in a memory chunk */

	fl_rectf(0, 0, 40, h(), COLOR_BG_MAIN);

	fl_line_style(FL_DASH, 0, nullptr);
	fl_font(FL_HELVETICA, 11);

	int octave = 9;

	for (int i=1; i<=MAX_NOTES+1; i++) {

		/* print key note label. C C# D D# E F F# G G# A A# B */

		char note[6];
		int  step = i % 12;

		switch (step) {
			case 1:
				fl_rectf(0, i*CELL_H, 40, CELL_H, 30, 30, 30);
				sprintf(note, "%dG", octave);
				break;
			case 2:
				sprintf(note, "%dF#", octave);
				break;
			case 3:
				sprintf(note, "%dF", octave);
				break;
			case 4:
				fl_rectf(0, i*CELL_H, 40, CELL_H, 30, 30, 30);
				sprintf(note, "%dE", octave);
				break;
			case 5:
				sprintf(note, "%dD#", octave);
				break;
			case 6:
				fl_rectf(0, i*CELL_H, 40, CELL_H, 30, 30, 30);
				sprintf(note, "%dD", octave);
				break;
			case 7:
				sprintf(note, "%dC#", octave);
				break;
			case 8:
				sprintf(note, "%dC", octave);
				break;
			case 9:
				fl_rectf(0, i*CELL_H, 40, CELL_H, 30, 30, 30);
				sprintf(note, "%dB", octave);
				break;
			case 10:
				sprintf(note, "%dA#", octave);
				break;
			case 11:
				fl_rectf(0, i*CELL_H, 40, CELL_H, 30, 30, 30);
				sprintf(note, "%dA", octave);
				break;
			case 0:
				sprintf(note, "%dG#", octave);
				octave--;
				break;
		}

		fl_color(fl_rgb_color(54, 54, 54));
		fl_draw(note, 4, ((i-1)*CELL_H)+1, 30, CELL_H, (Fl_Align) (FL_ALIGN_LEFT | FL_ALIGN_CENTER));

		/* print horizontal line */

		if (i < 128)
			fl_line(0, i*CELL_H, 40, +i*CELL_H);
	}

	fl_line_style(0);
	fl_end_offscreen();
}


/* -------------------------------------------------------------------------- */


void gePianoRoll::drawSurface2()
{
	surface2 = fl_create_offscreen(40, h());
	fl_begin_offscreen(surface2);
	fl_rectf(0, 0, 40, h(), COLOR_BG_MAIN);
	fl_color(fl_rgb_color(54, 54, 54));
	fl_line_style(FL_DASH, 0, nullptr);
	for (int i=1; i<=MAX_NOTES+1; i++) {
		int  step = i % 12;
		switch (step) {
			case 1:
			case 4:
			case 6:
			case 9:
			case 11:
				fl_rectf(0, i*CELL_H, 40, CELL_H, 30, 30, 30);
				break;
		}
		if (i < 128) {
			fl_color(fl_rgb_color(54, 54, 54));
			fl_line(0, i*CELL_H, 40, +i*CELL_H);
		}
	}
	fl_line_style(0);
	fl_end_offscreen();
}


/* -------------------------------------------------------------------------- */


void gePianoRoll::draw()
{
	fl_copy_offscreen(x(), y(), 40, h(), surface1, 0, 0);

#if defined(__APPLE__)
	for (int i=36; i<pParent->totalWidth; i+=36) /// TODO: i < pParent->coverX is faster
		fl_copy_offscreen(x()+i, y(), 40, h(), surface2, 1, 0);
#else
	for (int i=40; i<pParent->totalWidth; i+=40) /// TODO: i < pParent->coverX is faster
		fl_copy_offscreen(x()+i, y(), 40, h(), surface2, 0, 0);
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

				/* ax is driven by grid, ay by the height in px of each note */

				int ax = Fl::event_x();
				int ay = Fl::event_y();

				/* vertical snap */

				int edge = (ay-y()-3) % 15;
				if (edge != 0) ay -= edge;

				/* if no overlap, add new piano item. Also check that it doesn't
				 * overflow on the grey area, by shifting it to the left if
				 * necessary. */

				if (!onItem(ax, ay-y()-3)) {
					int greyover = ax+20 - pParent->coverX-x();
					if (greyover > 0)
						ax -= greyover;
					add(new gePianoItem(ax, ay, ax-x(), ay-y()-3, nullptr, nullptr, pParent));
					redraw();
				}
			}
			ret = 1;
			break;
		}
		case FL_DRAG:	{

			if (Fl::event_button3()) {

				geNoteEditor *prc = (geNoteEditor*) parent();
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


/* ------------------------------------------------------------------ */


void gePianoRoll::updateActions()
{
	/* when zooming, don't delete and re-add actions, just MOVE them. This
	 * function shifts the action by a zoom factor. Those singlepress are
	 * stretched, as well */

	gePianoItem *i;
	for (int k=0; k<children(); k++) {
		i = (gePianoItem*) child(k);

		//gu_log("found point %p, frame_a=%d frame_b=%d, x()=%d\n", (void*) i, i->getFrame_a(), i->getFrame_b(), i->x());

		int newX = x() + (i->getFrame_a() / pParent->zoom);
		int newW = ((i->getFrame_b() - i->getFrame_a()) / pParent->zoom);
		if (newW < 8)
			newW = 8;
		i->resize(newX, i->y(), newW, i->h());
		i->redraw();

		//gu_log("update point %p, frame_a=%d frame_b=%d, x()=%d\n", (void*) i, i->getFrame_a(), i->getFrame_b(), i->x());
	}
}


/* -------------------------------------------------------------------------- */


bool gePianoRoll::onItem(int rel_x, int rel_y)
{
	if (!pParent->chan->hasActions)
		return false;

	int note = MAX_NOTES - (rel_y / CELL_H);

	int n = children();
	for (int i=0; i<n; i++) {   // no scrollbars to skip

		gePianoItem *p = (gePianoItem*) child(i);
		if (p->getNote() != note)
			continue;

		/* when 2 segments overlap?
		 * start = the highest value between the two starting points
		 * end   = the lowest value between the two ending points
		 * if start < end then there's an overlap of end-start pixels. We
		 * also add 1 px to the edges in order to gain some space:
		 * [   ][   ]  ---> no
		 * [   ] [   ] ---> yes! */

		int start = p->x() > rel_x ? p->x() : rel_x-1;
		int end   = p->x()+p->w() < rel_x + 20 ? p->x()+p->w() : rel_x + 21;
		if (start < end)
			return true;
	}
	return false;
}
