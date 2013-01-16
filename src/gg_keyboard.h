/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gg_keyboard
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013 Giovanni A. Zuliani | Monocasual
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
 * ------------------------------------------------------------------ */

#ifndef GG_KEYBOARD_H
#define GG_KEYBOARD_H

#include <stdio.h>
#include <stdint.h>          // for intptr_t
#include <FL/Fl.H>
#include "gd_browser.h"
#include "const.h"
#include "mixer.h"
#include "gd_editor.h"


class Keyboard : public Fl_Group {
private:
	static void cb_button        (Fl_Widget *v, void *p);
	static void cb_openChanMenu  (Fl_Widget *v, void *p);
	static void cb_change_vol    (Fl_Widget *v, void *p);
	static void cb_mute          (Fl_Widget *v, void *p);
	static void cb_readActions   (Fl_Widget *v, void *p);
#ifdef WITH_VST
	static void cb_openFxWindow  (Fl_Widget *v, void *p);
#endif
	inline void __cb_button      (class gButton *gb);
	inline void __cb_openChanMenu(int chan);
	inline void __cb_change_vol  (int chan);
	inline void __cb_mute        (int chan);
	inline void __cb_readActions (int chan);
#ifdef WITH_VST
	inline void __cb_openFxWindow(int chan);
#endif

	bool bckspcPressed;
	bool endPressed;
	bool spacePressed;
	bool enterPressed;

public:
	class gStatus     *status      [MAX_NUM_CHAN];
	class gDial       *vol         [MAX_NUM_CHAN];
#ifdef WITH_VST
	class gButton     *fx          [MAX_NUM_CHAN];
#endif
	class gClick 	    *mute        [MAX_NUM_CHAN];
	class gClick 	    *readActions [MAX_NUM_CHAN];
	class gClick 	    *sampleButton[MAX_NUM_CHAN];
	class gModeBox    *modeBoxes   [MAX_NUM_CHAN];
	class gButton     *butts       [MAX_NUM_CHAN];

	Keyboard(int X,int Y,int W,int H,const char *L=0);
	int handle(int e);

	/* [add/rem]ActionButton
	 * add or removes the 'R' button. Status tells if the button should be
	 * on or off during the first appearance */

	void addActionButton(int i, bool status);
	void remActionButton(int i);
};

#endif
