/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 * ge_controller
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


#ifndef GE_CONTROLLER_H
#define GE_CONTROLLER_H


#include <FL/Fl_Group.H>


class gController : public Fl_Group
{
private:

	class gClick *rewind;
	class gClick *play;
	class gClick *recAction;
	class gClick *recInput;
	class gClick *metronome;

	static void cb_rewind   (Fl_Widget *v, void *p);
	static void cb_play     (Fl_Widget *v, void *p);
	static void cb_recAction(Fl_Widget *v, void *p);
	static void cb_recInput (Fl_Widget *v, void *p);
	static void cb_metronome(Fl_Widget *v, void *p);

	inline void __cb_rewind   ();
	inline void __cb_play     ();
	inline void __cb_recAction();
	inline void __cb_recInput ();
	inline void __cb_metronome();

public:

	gController(int x, int y);

	void updatePlay     (int v);
	void updateMetronome(int v);
	void updateRecInput (int v);
	void updateRecAction(int v);
};

#endif
