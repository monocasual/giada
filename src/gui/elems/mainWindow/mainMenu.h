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


#ifndef GE_MAIN_MENU_H
#define GE_MAIN_MENU_H


#include <FL/Fl_Group.H>


class geButton;


class geMainMenu : public Fl_Group
{
private:

  geButton *file;
  geButton *edit;
  geButton *config;
  geButton *about;

	static void cb_about (Fl_Widget *v, void *p);
	static void cb_config(Fl_Widget *v, void *p);
	static void cb_file  (Fl_Widget *v, void *p);
	static void cb_edit  (Fl_Widget *v, void *p);

	inline void __cb_about ();
	inline void __cb_config();
	inline void __cb_file  ();
	inline void __cb_edit  ();

public:

	geMainMenu(int x, int y);
};


#endif
