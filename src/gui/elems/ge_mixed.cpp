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
