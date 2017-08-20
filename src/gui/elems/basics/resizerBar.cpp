/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * geResizerBar
 * 'resizer bar' between widgets Fl_Scroll. Thanks to Greg Ercolano from
 * FLTK dev team. http://seriss.com/people/erco/fltk/
 *
 * Shows a resize cursor when hovered over.
 * Assumes:
 *     - Parent is an Fl_Scroll
 *     - All children of Fl_Scroll are m_vertically arranged
 *     - The widget above us has a bottom edge touching our top edge
 *       ie. (w->y()+w->h() == this->y())
 *
 * When this widget is dragged:
 *     - The widget above us (with a common edge) will be /resized/
 *       m_vertically
 *     - All children below us will be /moved/ m_vertically
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


#include <FL/Fl.H>
#include <FL/Fl_Scroll.H>
#include <FL/fl_draw.H>
#include "resizerBar.h"


geResizerBar::geResizerBar(int X, int Y, int W, int H, int minSize, bool type)
  : Fl_Box   (X, Y, W, H), 
    m_type   (type),
    m_minSize(minSize)
{
  m_lastPos = 0;
  if (m_type == VERTICAL) {
    m_origSize = H;
    labelsize(H);
  }
  else {
    m_origSize = W;
    labelsize(W);
  }
  align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
  labelfont(FL_COURIER);
  visible_focus(0);
}


/* -------------------------------------------------------------------------- */


void geResizerBar::handleDrag(int diff)
{
  Fl_Scroll* grp = static_cast<Fl_Scroll*>(parent());
  int top;
  int bot;
  if (m_type == VERTICAL) {
    top = y();
    bot = y()+h();
  }
  else {
    top = x();
    bot = x()+w();
  }

  // First pass: find widget directly above us with common edge
  //    Possibly clamp 'diff' if widget would get too small..

  for (int t=0; t<grp->children(); t++) {
    Fl_Widget* wd = grp->child(t);
    if (m_type == VERTICAL) {
      if ((wd->y()+wd->h()) == top) {                          // found widget directly above?
        if ((wd->h()+diff) < m_minSize)
          diff = wd->h() - m_minSize;                          // clamp
        wd->resize(wd->x(), wd->y(), wd->w(), wd->h()+diff);   // change height
        break;                                                 // done with first pass
      }
    }
    else {
      if ((wd->x()+wd->w()) == top) {                          // found widget directly above?
        if ((wd->w()+diff) < m_minSize)
          diff = wd->w() - m_minSize;                          // clamp
        wd->resize(wd->x(), wd->y(), wd->w()+diff, wd->h());   // change height
        break;                                                 // done with first pass
      }
    }
  }

  // Second pass: find widgets below us, move based on clamped diff

  for (int t=0; t<grp->children(); t++) {
    Fl_Widget* wd = grp->child(t);
    if (m_type == VERTICAL) {
      if (wd->y() >= bot)                                     // found widget below us?
        wd->resize(wd->x(), wd->y()+diff, wd->w(), wd->h());  // change position
    }
    else {
      if (wd->x() >= bot)
        wd->resize(wd->x()+diff, wd->y(), wd->w(), wd->h());
    }
  }

  // Change our position last

  if (m_type == VERTICAL)
    resize(x(), y()+diff, w(), h());
  else
    resize(x()+diff, y(), w(), h());

  grp->init_sizes();
  grp->redraw();
}


/* -------------------------------------------------------------------------- */


int geResizerBar::handle(int e)
{
  int ret = 0;
  int this_y;
  if (m_type == VERTICAL)
    this_y = Fl::event_y_root();
  else
    this_y = Fl::event_x_root();
  switch (e) {
    case FL_FOCUS:
      ret = 1;
      break;
    case FL_ENTER:
      ret = 1;
      fl_cursor(m_type == VERTICAL ? FL_CURSOR_NS : FL_CURSOR_WE);
      break;
    case FL_LEAVE:
      ret = 1;
      fl_cursor(FL_CURSOR_DEFAULT);
      break;
    case FL_PUSH:
      ret = 1;
      m_lastPos = this_y;
      break;
    case FL_DRAG:
      handleDrag(this_y-m_lastPos);
      m_lastPos = this_y;
      ret = 1;
      break;
    default: break;
  }
  return(Fl_Box::handle(e) | ret);
}


/* -------------------------------------------------------------------------- */


int geResizerBar::getMinSize() const
{ 
	return m_minSize; 
} 


/* -------------------------------------------------------------------------- */


void geResizerBar::resize(int x, int y, int w, int h)
{
	if (m_type == VERTICAL)
		Fl_Box::resize(x, y, w, m_origSize); // Height of resizer stays constant size
	else
		Fl_Box::resize(x, y, m_origSize, h);
}
