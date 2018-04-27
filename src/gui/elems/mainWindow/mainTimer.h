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


#ifndef GE_MAIN_TIMER_H
#define GE_MAIN_TIMER_H


#include <FL/Fl_Group.H>


class geButton;
class geChoice;


class geMainTimer : public Fl_Group
{
private:

	geButton* bpm;
	geButton* meter;
	geChoice* quantizer;
	geButton* multiplier;
	geButton* divider;

	static void cb_bpm       (Fl_Widget* v, void* p);
	static void cb_meter     (Fl_Widget* v, void* p);
	static void cb_quantizer (Fl_Widget* v, void* p);
	static void cb_multiplier(Fl_Widget* v, void* p);
	static void cb_divider   (Fl_Widget* v, void* p);
	inline void cb_bpm();
	inline void cb_meter();
	inline void cb_quantizer();
	inline void cb_multiplier();
	inline void cb_divider();

public:

	geMainTimer(int x, int y);

	void setBpm(const char* v);
	void setBpm(float v);
	void setMeter(int beats, int bars);
	void setQuantizer(int q);

  /* setLock
  Locks bpm, beter and multipliers. Used during audio recordings. */

  void setLock(bool v);
};


#endif
