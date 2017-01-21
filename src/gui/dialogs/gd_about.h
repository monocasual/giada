/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_about
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


#ifndef GD_ABOUT_H
#define GD_ABOUT_H


#include "../elems/ge_window.h"


class gdAbout : public gWindow
{
private:

	class gBox 	 *logo;
	class gBox 	 *text;
	class gClick *close;

#ifdef WITH_VST
	class gBox  *vstText;
	class gBox  *vstLogo;
#endif

public:

	gdAbout();
	~gdAbout();

	static void cb_close(Fl_Widget *w, void *p);
	inline void __cb_close();
};

#endif
