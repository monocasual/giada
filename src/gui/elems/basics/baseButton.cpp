/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * geBaseButton
 * Base class for every button widget.
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


#include <FL/fl_draw.H>
#include "baseButton.h"


geBaseButton::geBaseButton(int x, int y, int w, int h, const char *l)
  : Fl_Button(x, y, w, h, l)
{
  initLabel = l ? l : "";
}


/* -------------------------------------------------------------------------- */


void geBaseButton::trimLabel()
{
  if (initLabel.empty())
    return;

  std::string out;
  if (w() > 20) {
    out = initLabel;
    int len = initLabel.size();
    while (fl_width(out.c_str(), out.size()) > w()) {
      out = initLabel.substr(0, len) + "...";
      len--;
    }
  }
  else {
    out = "";
  }
  copy_label(out.c_str());
}


/* -------------------------------------------------------------------------- */


void geBaseButton::label(const char *l)
{
  Fl_Button::label(l);
  initLabel = l;
  trimLabel();
}


const char *geBaseButton::label()
{
  return Fl_Button::label();
}


/* -------------------------------------------------------------------------- */


void geBaseButton::resize(int X, int Y, int W, int H)
{
  trimLabel();
  Fl_Button::resize(X, Y, W, H);
}
