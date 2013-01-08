/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_actionChannel
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2012 Giovanni A. Zuliani | Monocasual
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
 * ------------------------------------------------------------------ */


#include "ge_actionChannel.h"


extern Mixer    G_Mixer;
extern Conf	    G_Conf;


/* ------------------------------------------------------------------ */


gActionChannel::gActionChannel(int x, int y, gdActionEditor *parent)
 : gActionWidget(x, y, 200, 40, parent), selected(NULL)
{
	/* let's add actions when the window opens. Their position is zoom-based;
	 * each frame is / 2 because we don't care about stereo infos. */

	for (unsigned i=0; i<recorder::frames.size; i++) {
		for (unsigned j=0; j<recorder::global.at(i).size; j++) {
			if (recorder::global.at(i).at(j)->chan == parent->chan) {

				/* don't show actions > than the grey area */

				if (recorder::frames.at(i) > G_Mixer.totalFrames)
					continue;

				/* single_press channel? be careful: the nex action is the key_release, i.e.
				 * the end of a composite action (press+release). In that case, skip it to
				 * the next event. */

				if (G_Mixer.chanMode[parent->chan] == SINGLE_PRESS &&
				    recorder::global.at(i).at(j)->type == ACTION_KEYREL)
				{
					continue;
				}

				/* skip the killchan actions in a singlepress channel. They cannot be recorded
				 * in such mode, but they can exist if you change from another mode to singlepress */

				if (recorder::global.at(i).at(j)->type == ACTION_KILLCHAN &&
						G_Mixer.chanMode[parent->chan] == SINGLE_PRESS)
				{
					continue;
				}

				/* we also skip mutes. There's a widget for that */

				if (recorder::global.at(i).at(j)->type & (ACTION_MUTEON | ACTION_MUTEOFF))
					continue;

				int ax = x+((recorder::frames.at(i)/2)/parent->zoom);
				gAction *a = new gAction(
						ax,                                   // x
						y+4,                                  // y
						h()-8,                                // h
						recorder::frames.at(i),               // actual frame
						i,                                    // n. of recordings
						parent,                               // pointer to the parent window
						false,                                // record = false: don't record it, we are just displaying it!
						recorder::global.at(i).at(j)->type);  // type of action
				add(a);
			}
		}
	}
	end(); // mandatory when you add widgets to a fl_group, otherwise mega malfunctions
}


/* ------------------------------------------------------------------ */


gAction *gActionChannel::getSelectedAction() {
	for (int i=0; i<children(); i++) {
		int action_x  = ((gAction*)child(i))->x();
		int action_w  = ((gAction*)child(i))->w();
		if (Fl::event_x() >= action_x && Fl::event_x() <= action_x + action_w)
			return (gAction*)child(i);
	}
	return NULL;
}


/* ------------------------------------------------------------------ */


void gActionChannel::updateActions() {

	/* when zooming, don't delete and re-add actions, just MOVE them. This
	 * function shifts the action by a zoom factor. Those singlepress are
	 * stretched, as well */

	gAction *a;
	for (int i=0; i<children(); i++) {

		a = (gAction*)child(i);
		int newX = x() + ((a->frame_a/2) / parent->zoom);

		if (G_Mixer.chanMode[parent->chan] == SINGLE_PRESS) {
			int newW = ((a->frame_b - a->frame_a) / 2 / parent->zoom);
			if (newW < gAction::MIN_WIDTH)
				newW = gAction::MIN_WIDTH;
			a->resize(newX, a->y(), newW, a->h());
		}
		else
			a->position(newX, a->y());
	}
}


/* ------------------------------------------------------------------ */


void gActionChannel::draw() {

	/* draw basic boundaries (+ beat bars) and hide the unused area. Then
	 * draw the children (the actions) */

	baseDraw();
	fl_rectf(parent->coverX, y()+1, parent->totalWidth-parent->coverX+x(), h()-2, COLOR_BG_1);
	draw_children();
}


/* ------------------------------------------------------------------ */


int gActionChannel::handle(int e) {

	int ret = Fl_Group::handle(e);

	/* do nothing if the widget is deactivated. It could happen for loopmode
	 * channels */

	if (!active())
		return 1;

	switch (e) {

		case FL_DRAG: {
			if (selected != NULL) {   // if you don't drag an empty area

				/* if onLeftEdge o onRightEdge are true it means that you're resizing
				 * an action. Otherwise move the widget. */

				if (selected->onLeftEdge || selected->onRightEdge) {

					/* some checks: a) cannot resize an action < N pixels, b) no beyond zero,
					 * c) no beyond bar maxwidth. Checks for overlap are done in FL_RELEASE */

					if (selected->onRightEdge) {

						int aw = Fl::event_x()-selected->x();
						int ah = selected->h();

						if (Fl::event_x() < selected->x()+gAction::MIN_WIDTH)
							aw = gAction::MIN_WIDTH;
						else
						if (Fl::event_x() > parent->coverX)
							aw = parent->coverX-selected->x();

						selected->size(aw, ah);
					}
					else {

						int ax = Fl::event_x();
						int ay = selected->y();
						int aw = selected->x()-Fl::event_x()+selected->w();
						int ah = selected->h();

						if (Fl::event_x() < x()) {
							ax = x();
							aw = selected->w()+selected->x()-x();
						}
						else
						if (Fl::event_x() > selected->x()+selected->w()-gAction::MIN_WIDTH) {
							ax = selected->x()+selected->w()-gAction::MIN_WIDTH;
							aw = gAction::MIN_WIDTH;
						}
						selected->resize(ax, ay, aw, ah);
					}
				}

				/* move the widget around */

				else {
					int real_x = Fl::event_x() - actionPickPoint;
					if (real_x < x())                                  // don't go beyond the left border
						selected->position(x(), selected->y());
					else
					if (real_x+selected->w() > parent->coverX)         // don't go beyond the right border
						selected->position(parent->coverX-selected->w(), selected->y());
					else {
						if (parent->gridTool->isOn()) {
							puts("TODO snapping");
						}
						else
							selected->position(real_x, selected->y());
					}
				}
				redraw();
			}
			ret = 1;
			break;
		}

		case FL_PUSH:	{

			if (Fl::event_button1()) {

				/* avoid at all costs two overlapping actions. We use 'selected' because
				 * the selected action can be reused in FL_DRAG, in case you want to move
				 * it. */

				selected = getSelectedAction();

				if (selected == NULL) {

					/* avoid click on grey area */

					if (Fl::event_x() >= parent->coverX) {
						ret = 1;
						break;
					}

					gAction *a = new gAction(
							Fl::event_x(),                        // x
							y()+4,                                // y
							h()-8,                                // h
							((Fl::event_x()-x())*parent->zoom)*2, // actual frame
							recorder::frames.size-1,              // n. of actions recorded
							parent,                               // parent window pointer
							true,                                 // record = true: record it!
							parent->getActionType());             // type of action
					add(a);
					glue_setChannelWithActions(parent->chan); // mainWindow update
					redraw();
					ret = 1;
				}
				else {
					actionOriginalX = selected->x();
					actionOriginalW = selected->w();
					actionPickPoint = Fl::event_x() - selected->x();
					ret = 1;   // for dragging
				}
			}
			else
			if (Fl::event_button3()) {
				gAction *a = getSelectedAction();
				if (a != NULL) {
					a->delAction();
					remove(a);
					delete a;
					glue_setChannelWithActions(parent->chan);
					redraw();
					ret = 1;
				}
			}
			break;
		}
		case FL_RELEASE: {

			if (selected == NULL) {
				ret = 1;
				break;
			}

			/* noChanges = true when you click on an action without doing anything
			 * (dragging or moving it). */

			bool noChanges = false;
			if (actionOriginalX == selected->x())
				noChanges = true;
			if (G_Mixer.chanMode[parent->chan] == SINGLE_PRESS &&
					actionOriginalX+actionOriginalW != selected->x()+selected->w())
				noChanges = false;

			if (noChanges) {
				ret = 1;
				selected = NULL;
				break;
			}

			/* step 1: check if the action doesn't overlap with another one.
			 * In case of overlap the moved action returns to the original X
			 * value ("actionOriginalX"). */

			bool overlap = false;
			for (int i=0; i<children() && !overlap; i++) {

				/* never check against itself. */

				if ((gAction*)child(i) == selected)
					continue;

				int action_x  = ((gAction*)child(i))->x();
				int action_w  = ((gAction*)child(i))->w();
				if (G_Mixer.chanMode[parent->chan] == SINGLE_PRESS) {

					/* when 2 segments overlap?
					 * start = the highest value between the two starting points
					 * end   = the lowest value between the two ending points
					 * if start < end then there's an overlap of end-start pixels. */

					 int start = action_x >= selected->x() ? action_x : selected->x();
					 int end   = action_x+action_w < selected->x()+selected->w() ? action_x+action_w : selected->x()+selected->w();
					 if (start < end) {
						selected->resize(actionOriginalX, selected->y(), actionOriginalW, selected->h());
						redraw();
						overlap = true;   // one overlap: stop checking
					}
				}
				else {
					if (selected->x() == action_x) {
						selected->position(actionOriginalX, selected->y());
						redraw();
						overlap = true;   // one overlap: stop checking
					}
				}
			}

			/* step 2: no overlap? then update the coordinates of the action, ie
			 * delete the previous rec and create a new one.
			 * Anyway the selected action becomes NULL, because when you release
			 * the mouse button the dragging process ends. */

			if (!overlap)
				selected->moveAction();
			selected = NULL;
			ret = 1;
			break;
		}
	}

	return ret;
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


const int gAction::MIN_WIDTH = 8;


/* ------------------------------------------------------------------ */


/** index is useless? */
gAction::gAction(int x, int y, int h, unsigned frame_a, unsigned index, gdActionEditor *parent, bool record, char type)
: Fl_Box     (x, y, MIN_WIDTH, h),
  selected   (false),
  index      (index),
  parent     (parent),
  type       (type),
  frame_a    (frame_a),
  onRightEdge(false),
  onLeftEdge (false)
{
	/* bool 'record' defines how to understand the action.
	 * record = false: don't record it, just show it. It happens when you
	 * open the editor with some actions to be shown.
	 *
	 * record = true: record it AND show it. It happens when you click on
	 * an empty area in order to add a new actions. First you record it
	 * (addAction()) then you show it (FLTK::Draw()) */

	if (record)
		addAction();

	/* in order to show a singlepress action we must compute the frame_b. We
	 * do that after the possible recording, otherwise we don't know which
	 * key_release is associated. */

	if (G_Mixer.chanMode[parent->chan] == SINGLE_PRESS) {
		frame_b = recorder::getEndActionFrame(parent->chan, ACTION_KEYREL, frame_a);
		if (frame_b == -1)
			frame_b = frame_a+4096;
		w((frame_b - frame_a)/parent->zoom/2);
	}
}


/* ------------------------------------------------------------------ */


void gAction::draw() {

	/* a singlepress action narrower than 8 pixel is useless. So check it.
	 * Warning: if an action is 8 px narrow, it has no body space to drag
	 * it. It's up to the user to zoom in and drag it. */


	if (G_Mixer.chanMode[parent->chan] == SINGLE_PRESS)
		if (w() < MIN_WIDTH)
			w(MIN_WIDTH);


	int color;
	if (selected)  /// && gActionChannel !disabled
		color = COLOR_BD_1;
	else
		color = COLOR_BG_2;

	if (type == ACTION_KILLCHAN)
		fl_rect(x(), y(), w(), h(), (Fl_Color) color);
	else {
		fl_rectf(x(), y(), w(), h(), (Fl_Color) color);
		if (G_Mixer.chanMode[parent->chan] != SINGLE_PRESS) { // don't do that for SINGLE PRESS
			if (type == ACTION_KEYPRESS)
				fl_rectf(x()+3, y()+h()-11, 2, 8, COLOR_BD_0);
			else
			if  (type == ACTION_KEYREL)
				fl_rectf(x()+3, y()+3, 2, 8, COLOR_BD_0);
		}
	}

}


/* ------------------------------------------------------------------ */


int gAction::handle(int e) {

	/* ret = 0 sends the event to the parent window. */

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

			/* handling of the two margins, left & right. 4 pixels are good enough */

			if (G_Mixer.chanMode[parent->chan] == SINGLE_PRESS) {
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
			}
		}
	}

	return ret;
}


/* ------------------------------------------------------------------ */


void gAction::addAction() {

	/* anatomy of an action
	 * ____[#######]_____ (a) is the left margin, ACTION_KEYPRESS. (b) is
	 *     a       b      the right margin, the ACTION_KEYREL. This is the
	 * theory behind the singleshot.press actions; for any other kind the
	 * (b) is just a graphical and meaningless point. */

	if (G_Mixer.chanMode[parent->chan] == SINGLE_PRESS) {
		recorder::rec(parent->chan, ACTION_KEYPRESS, frame_a);
		recorder::rec(parent->chan, ACTION_KEYREL, frame_a+4096);
		//printf("action added, [%d, %d]\n", frame_a, frame_a+4096);
	}
	else {
		recorder::rec(parent->chan, parent->getActionType(), frame_a);
		//printf("action added, [%d]\n", frame_a);
	}

	recorder::sortActions();

	index++; // important!
}


/* ------------------------------------------------------------------ */


void gAction::delAction() {

	/* if SINGLE_PRESS you must delete both the keypress and the keyrelease
	 * actions. */

	if (G_Mixer.chanMode[parent->chan] == SINGLE_PRESS) {
		recorder::deleteAction(parent->chan, frame_a, ACTION_KEYPRESS);
		recorder::deleteAction(parent->chan, frame_b, ACTION_KEYREL);
	}
	else
		recorder::deleteAction(parent->chan, frame_a, type);

	/* restore the initial cursor shape, in case you delete an action and
	 * the double arrow (for resizing) is displayed */

	fl_cursor(FL_CURSOR_DEFAULT, FL_WHITE, FL_BLACK);
}


/* ------------------------------------------------------------------ */


void gAction::moveAction() {

	/* easy one: delete previous action and record the new ones. As usual,
	 * SINGLE_PRESS requires two jobs */

	delAction();

	/* frame update. The offset of gActionChannel is calculated by going up
	 * to the parent ( x()-parent->ac->x() ) */

	frame_a = (x()-parent->ac->x())*parent->zoom*2;
	recorder::rec(parent->chan, type, frame_a);

	if (G_Mixer.chanMode[parent->chan] == SINGLE_PRESS) {
		frame_b = (x()+w()-parent->ac->x())*parent->zoom*2;;
		recorder::rec(parent->chan, ACTION_KEYREL, frame_b);
	}

	recorder::sortActions();
}
