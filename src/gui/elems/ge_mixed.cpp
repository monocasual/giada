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


#include <cmath>
#include "../../core/const.h"
#include "../../core/mixer.h"
#include "../../core/graphics.h"
#include "../../core/recorder.h"
#include "../../core/channel.h"
#include "../../core/kernelAudio.h"
#include "../../core/sampleChannel.h"
#include "../../utils/gui.h"
#include "../dialogs/gd_mainWindow.h"
#include "basics/boxtypes.h"
#include "ge_mixed.h"


extern gdMainWindow *mainWin;


using namespace giada;


void __cb_window_closer(Fl_Widget *v, void *p)
{
  delete (Fl_Window*)p;
}


/* -------------------------------------------------------------------------- */


gInput::gInput(int x, int y, int w, int h, const char *L)
: Fl_Input(x, y, w, h, L)
{
  //Fl::set_boxtype(G_CUSTOM_BORDER_BOX, gDrawBox, 1, 1, 2, 2);
  box(G_CUSTOM_BORDER_BOX);
  labelsize(GUI_FONT_SIZE_BASE);
  labelcolor(COLOR_TEXT_0);
  color(COLOR_BG_DARK);
  textcolor(COLOR_TEXT_0);
  cursor_color(COLOR_TEXT_0);
  selection_color(COLOR_BD_0);
  textsize(GUI_FONT_SIZE_BASE);
}


/* -------------------------------------------------------------------------- */


gDial::gDial(int x, int y, int w, int h, const char *L)
: Fl_Dial(x, y, w, h, L)
{
  labelsize(GUI_FONT_SIZE_BASE);
  labelcolor(COLOR_TEXT_0);
  align(FL_ALIGN_LEFT);
  type(FL_FILL_DIAL);
  angles(0, 360);
  color(COLOR_BG_0);            // background
  selection_color(COLOR_BG_1);   // selection
}

void gDial::draw()
{
  double angle = (angle2()-angle1())*(value()-minimum())/(maximum()-minimum()) + angle1();

  fl_color(COLOR_BG_0);
  fl_pie(x(), y(), w(), h(), 270-angle1(), angle > angle1() ? 360+270-angle : 270-360-angle);

  fl_color(COLOR_BD_0);
  fl_arc(x(), y(), w(), h(), 0, 360);
  fl_pie(x(), y(), w(), h(), 270-angle, 270-angle1());
}

/* -------------------------------------------------------------------------- */


gBox::gBox(int x, int y, int w, int h, const char *L, Fl_Align al)
: Fl_Box(x, y, w, h)
{
  copy_label(L);
  labelsize(GUI_FONT_SIZE_BASE);
  box(FL_NO_BOX);
  labelcolor(COLOR_TEXT_0);
  if (al != 0)
    align(al | FL_ALIGN_INSIDE);
}


/* -------------------------------------------------------------------------- */


gCheck::gCheck(int x, int y, int w, int h, const char *L)
: Fl_Check_Button(x, y, w, h, L) {}

void gCheck::draw()
{
  int color = !active() ? FL_INACTIVE_COLOR : COLOR_BD_0;

  if (value()) {
    fl_rect(x(), y(), 12, 12, (Fl_Color) color);
    fl_rectf(x(), y(), 12, 12, (Fl_Color) color);
  }
  else {
    fl_rectf(x(), y(), 12, 12, FL_BACKGROUND_COLOR);
    fl_rect(x(), y(), 12, 12, (Fl_Color) color);
  }

  fl_rectf(x()+20, y(), w(), h(), FL_BACKGROUND_COLOR);  // clearer
  fl_font(FL_HELVETICA, GUI_FONT_SIZE_BASE);
  fl_color(!active() ? FL_INACTIVE_COLOR : COLOR_TEXT_0);
  fl_draw(label(), x()+20, y(), w(), h(), (Fl_Align) (FL_ALIGN_LEFT | FL_ALIGN_TOP));
}


/* -------------------------------------------------------------------------- */


gRadio::gRadio(int x, int y, int w, int h, const char *L)
: Fl_Radio_Button(x, y, w, h, L) {}

void gRadio::draw()
{
  int color = !active() ? FL_INACTIVE_COLOR : COLOR_BD_0;

  if (value()) {
    fl_rect(x(), y(), 12, 12, (Fl_Color) color);
    fl_rectf(x(), y(), 12, 12, (Fl_Color) color);
  }
  else {
    fl_rectf(x(), y(), 12, 12, FL_BACKGROUND_COLOR);
    fl_rect(x(), y(), 12, 12, (Fl_Color) color);
  }

  fl_rectf(x()+20, y(), w(), h(), FL_BACKGROUND_COLOR);  // clearer
  fl_font(FL_HELVETICA, GUI_FONT_SIZE_BASE);
  fl_color(COLOR_TEXT_0);
  fl_draw(label(), x()+20, y(), w(), h(), (Fl_Align) (FL_ALIGN_LEFT | FL_ALIGN_TOP));
}


/* -------------------------------------------------------------------------- */


gProgress::gProgress(int x, int y, int w, int h, const char *L)
: Fl_Progress(x, y, w, h, L) {
  color(COLOR_BG_0, COLOR_BD_0);
  box(G_CUSTOM_BORDER_BOX);

}


/* -------------------------------------------------------------------------- */


gChoice::gChoice(int x, int y, int w, int h, const char *l, bool ang)
  : Fl_Choice(x, y, w, h, l), angle(ang)
{
  labelsize(GUI_FONT_SIZE_BASE);
  labelcolor(COLOR_TEXT_0);
  box(FL_BORDER_BOX);
  textsize(GUI_FONT_SIZE_BASE);
  textcolor(COLOR_TEXT_0);
  color(COLOR_BG_0);
}


void gChoice::draw()
{
  fl_rectf(x(), y(), w(), h(), COLOR_BG_0);              // bg
  fl_rect(x(), y(), w(), h(), (Fl_Color) COLOR_BD_0);    // border
  if (angle)
    fl_polygon(x()+w()-8, y()+h()-1, x()+w()-1, y()+h()-8, x()+w()-1, y()+h()-1);

  /* pick up the text() from the selected item (value()) and print it in
   * the box and avoid overflows */

  fl_color(!active() ? COLOR_BD_0 : COLOR_TEXT_0);
  if (value() != -1) {
    if (fl_width(text(value())) < w()-8) {
      fl_draw(text(value()), x(), y(), w(), h(), FL_ALIGN_CENTER);
    }
    else {
      std::string tmp = text(value());
      int size        = tmp.size();
      while (fl_width(tmp.c_str()) >= w()-16) {
        tmp.resize(size);
        size--;
      }
      tmp += "...";
      fl_draw(tmp.c_str(), x(), y(), w(), h(), FL_ALIGN_CENTER);
    }

  }
}


/* -------------------------------------------------------------------------- */


gLiquidScroll::gLiquidScroll(int x, int y, int w, int h, const char *l)
  : Fl_Scroll(x, y, w, h, l)
{
  type(Fl_Scroll::VERTICAL);
  scrollbar.color(COLOR_BG_0);
  scrollbar.selection_color(COLOR_BG_1);
  scrollbar.labelcolor(COLOR_BD_1);
  scrollbar.slider(G_CUSTOM_BORDER_BOX);
}


void gLiquidScroll::resize(int X, int Y, int W, int H)
{
  int nc = children()-2;                // skip hscrollbar and vscrollbar
  for ( int t=0; t<nc; t++) {           // tell children to resize to our new width
    Fl_Widget *c = child(t);
    c->resize(c->x(), c->y(), W-24, c->h());    // W-24: leave room for scrollbar
  }
  init_sizes();   // tell scroll children changed in size
  Fl_Scroll::resize(X,Y,W,H);
}


/* -------------------------------------------------------------------------- */


gSlider::gSlider(int x, int y, int w, int h, const char *l)
  : Fl_Slider(x, y, w, h, l)
{
  type(FL_HOR_FILL_SLIDER);

  labelsize(GUI_FONT_SIZE_BASE);
  align(FL_ALIGN_LEFT);
  labelcolor(COLOR_TEXT_0);

  box(G_CUSTOM_BORDER_BOX);
  color(COLOR_BG_0);
  selection_color(COLOR_BD_0);
}
