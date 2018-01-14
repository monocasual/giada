/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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


#ifndef GE_PAN_TOOL_H
#define GE_PAN_TOOL_H


#include <FL/Fl_Group.H>


class SampleChannel;
class geDial;
class geInput;
class geButton;
class geBox;


class gePanTool : public Fl_Group
{
private:

  SampleChannel *ch;

  geBox    *label;
  geDial   *dial;
  geInput  *input;
  geButton *reset;

  static void cb_panning (Fl_Widget *w, void *p);
  static void cb_panReset(Fl_Widget *w, void *p);
  inline void __cb_panning();
  inline void __cb_panReset();

public:

  gePanTool(int x, int y, SampleChannel *ch);

  void refresh();
};


#endif
