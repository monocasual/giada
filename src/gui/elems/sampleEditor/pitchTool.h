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


#ifndef GE_PITCH_TOOL_H
#define GE_PITCH_TOOL_H


#include <FL/Fl_Group.H>


class SampleChannel;
class geDial;
class geInput;
class geButton;
class geBox;


class gePitchTool : public Fl_Group
{
private:

  SampleChannel *ch;

  geBox    *label;
  geDial   *dial;
  geInput  *input;
  geButton *pitchToBar;
  geButton *pitchToSong;
  geButton *pitchHalf;
  geButton *pitchDouble;
  geButton *pitchReset;

  static void cb_setPitch      (Fl_Widget *w, void *p);
  static void cb_setPitchToBar (Fl_Widget *w, void *p);
  static void cb_setPitchToSong(Fl_Widget *w, void *p);
  static void cb_setPitchHalf  (Fl_Widget *w, void *p);
  static void cb_setPitchDouble(Fl_Widget *w, void *p);
  static void cb_resetPitch    (Fl_Widget *w, void *p);
  static void cb_setPitchNum   (Fl_Widget *w, void *p);
  inline void __cb_setPitch();
  inline void __cb_setPitchToBar();
  inline void __cb_setPitchToSong();
  inline void __cb_setPitchHalf();
  inline void __cb_setPitchDouble();
  inline void __cb_resetPitch();
  inline void __cb_setPitchNum();

public:

  gePitchTool(int x, int y, SampleChannel *ch);

  void refresh();
};


#endif
