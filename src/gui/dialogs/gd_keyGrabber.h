/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_keyGrabber
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


#ifndef GD_KEYGRABBER_H
#define GD_KEYGRABBER_H


#include <FL/Fl.H>
#include "../elems/ge_window.h"


class gdKeyGrabber : public gWindow
{
private:

	class Channel *ch;

	class gBox   *text;
	class gClick *clear;
	class gClick *cancel;

	static void cb_clear (Fl_Widget *w, void *p);
	static void cb_cancel(Fl_Widget *w, void *p);
	inline void __cb_clear ();
	inline void __cb_cancel();

	void setButtonLabel(int key);

	void updateText(int key);

public:

	gdKeyGrabber(class Channel *ch);
	int handle(int e);
};

#endif
