/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_channelButton
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2015 Giovanni A. Zuliani | Monocasual
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


#include "../../core/const.h"
#include "../../utils/utils.h"
#include "ge_channelButton.h"


using std::string;


gChannelButton::gChannelButton(int x, int y, int w, int h, const char *l)
  : gClick(x, y, w, h, l), key("") {}


/* -------------------------------------------------------------------------- */


void gChannelButton::setKey(const string &k)
{
  key = k;
}


/* -------------------------------------------------------------------------- */


void gChannelButton::setKey(int k)
{
  if (k == 0)
    key = "";
  else {
    // FIXME - this crap won't work with unicode/utf-8
    char c = (char) k;
    key = c;
  }
}


/* -------------------------------------------------------------------------- */


void gChannelButton::draw()
{
  gClick::draw();

  if (key == "")
    return;

  /* draw background */

  fl_rectf(x()+1, y()+1, 18, h()-2, bgColor0);

  /* draw key */

  fl_color(COLOR_TEXT_0);
  fl_font(FL_HELVETICA, 11);
  fl_draw(key.c_str(), x(), y(), 18, h(), FL_ALIGN_CENTER);
}


/* -------------------------------------------------------------------------- */


void gChannelButton::setInputRecordMode()
{
  bgColor0 = COLOR_BG_3;
}


/* -------------------------------------------------------------------------- */


void gChannelButton::setActionRecordMode()
{
  bgColor0 = COLOR_BG_4;
  txtColor = COLOR_TEXT_0;
}


/* -------------------------------------------------------------------------- */


void gChannelButton::setDefaultMode(const char *l)
{
  bgColor0 = COLOR_BG_0;
	bdColor  = COLOR_BD_0;
	txtColor = COLOR_TEXT_0;
  if (l)
    label(l);
}


/* -------------------------------------------------------------------------- */


void gChannelButton::setPlayMode()
{
  bgColor0 = COLOR_BG_2;
  bdColor  = COLOR_BD_1;
  txtColor = COLOR_TEXT_1;
}


/* -------------------------------------------------------------------------- */


void gChannelButton::setEndingMode()
{
  bgColor0 = COLOR_BD_0;
}
