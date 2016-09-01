/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_modeBox
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


#include "../../utils/gui.h"
#include "../../core/graphics.h"
#include "../../core/sampleChannel.h"
#include "../../core/const.h"
#include "../dialogs/gd_mainWindow.h"
#include "ge_modeBox.h"


gModeBox::gModeBox(int x, int y, int w, int h, SampleChannel *ch, const char *L)
  : Fl_Menu_Button(x, y, w, h, L), ch(ch)
{
  box(G_BOX);
  textsize(GUI_FONT_SIZE_BASE);
  textcolor(COLOR_TEXT_0);
  color(COLOR_BG_0);

  add("Loop . basic",      0, cb_changeMode, (void *)LOOP_BASIC);
  add("Loop . once",       0, cb_changeMode, (void *)LOOP_ONCE);
  add("Loop . once . bar", 0, cb_changeMode, (void *)LOOP_ONCE_BAR);
  add("Loop . repeat",     0, cb_changeMode, (void *)LOOP_REPEAT);
  add("Oneshot . basic",   0, cb_changeMode, (void *)SINGLE_BASIC);
  add("Oneshot . press",   0, cb_changeMode, (void *)SINGLE_PRESS);
  add("Oneshot . retrig",  0, cb_changeMode, (void *)SINGLE_RETRIG);
  add("Oneshot . endless", 0, cb_changeMode, (void *)SINGLE_ENDLESS);
}


/* -------------------------------------------------------------------------- */


void gModeBox::draw() {
  fl_rect(x(), y(), w(), h(), COLOR_BD_0);    // border
  switch (ch->mode) {
    case LOOP_BASIC:
      fl_draw_pixmap(loopBasic_xpm, x()+1, y()+1);
      break;
    case LOOP_ONCE:
      fl_draw_pixmap(loopOnce_xpm, x()+1, y()+1);
      break;
    case LOOP_ONCE_BAR:
      fl_draw_pixmap(loopOnceBar_xpm, x()+1, y()+1);
      break;
    case LOOP_REPEAT:
      fl_draw_pixmap(loopRepeat_xpm, x()+1, y()+1);
      break;
    case SINGLE_BASIC:
      fl_draw_pixmap(oneshotBasic_xpm, x()+1, y()+1);
      break;
    case SINGLE_PRESS:
      fl_draw_pixmap(oneshotPress_xpm, x()+1, y()+1);
      break;
    case SINGLE_RETRIG:
      fl_draw_pixmap(oneshotRetrig_xpm, x()+1, y()+1);
      break;
    case SINGLE_ENDLESS:
      fl_draw_pixmap(oneshotEndless_xpm, x()+1, y()+1);
      break;
  }
}


/* -------------------------------------------------------------------------- */


void gModeBox::cb_changeMode(Fl_Widget *v, void *p) { ((gModeBox*)v)->__cb_changeMode((intptr_t)p); }


/* -------------------------------------------------------------------------- */


void gModeBox::__cb_changeMode(int mode)
{
  ch->mode = mode;

  /* what to do when the channel is playing and you change the mode?
   * Nothing, since v0.5.3. Just refresh the action editor window, in
   * case it's open */

  gu_refreshActionEditor();
}
