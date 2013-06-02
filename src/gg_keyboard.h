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
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Group.H>
#include "const.h"
#include "utils.h"


class gChannel : public Fl_Group {
private:
	static void cb_button        (Fl_Widget *v, void *p);
	static void cb_mute          (Fl_Widget *v, void *p);
	static void cb_solo          (Fl_Widget *v, void *p);
	static void cb_openChanMenu  (Fl_Widget *v, void *p);
	static void cb_change_vol    (Fl_Widget *v, void *p);
	static void cb_readActions   (Fl_Widget *v, void *p);
#ifdef WITH_VST
	static void cb_openFxWindow  (Fl_Widget *v, void *p);
#endif
	inline void __cb_button      ();
	inline void __cb_mute        ();
	inline void __cb_solo        ();
	inline void __cb_openChanMenu();
	inline void __cb_change_vol  ();
	inline void __cb_readActions ();
#ifdef WITH_VST
	inline void __cb_openFxWindow();
#endif

	void openBrowser(int type);

public:
	gChannel(int x, int y, int w, int h, const char *l, struct channel *ch);
	void reset();

	/* [add/rem]ActionButton
	 * add or removes the 'R' button. Status tells if the button should be
	 * on or off during the first appearance */

	void addActionButton(bool status);
	void remActionButton();
	int  keypress(int event);

	class gStatus     *status;
	class gDial       *vol;
	class gClick 	    *mute;
	class gClick 	    *solo;
	class gClick 	    *readActions;
	class gClick 	    *sampleButton;
	class gModeBox    *modeBox;
	class gButton     *button;
#ifdef WITH_VST
	class gButton     *fx;
#endif

	int key;
	struct channel *ch;
};


/* ------------------------------------------------------------------ */


class Keyboard : public Fl_Scroll {
private:
	static void cb_addChannelL  (Fl_Widget *v, void *p);
	static void cb_addChannelR  (Fl_Widget *v, void *p);
	inline void __cb_addChannelL();
	inline void __cb_addChannelR();

	/* fixRightColumn
	 * shift right column if scrollbar doesn't show up */

	void fixRightColumn();

	int openChanTypeMenu();

	bool bckspcPressed;
	bool endPressed;
	bool spacePressed;
	bool enterPressed;

	class gClick *addChannelL;
	class gClick *addChannelR;

public:
	Keyboard(int X,int Y,int W,int H,const char *L=0);
	int handle(int e);

	/* addChannel
	 * add a new channel to gChannels[l/r]. Used by callbacks and during
	 * patch loading. */

	gChannel *addChannel(char side, struct channel *ch);

	/* deleteChannel
	 * delete a channel from gChannels<> where gChannel->ch == ch and remove
	 * it from the stack. */

	void deleteChannel(struct channel *ch);

	/* freeChannel
	 * free a channel from gChannels<> where gChannel->ch == ch. No channels
	 * are deleted */

	void freeChannel(struct channel *ch);

	void updateChannel(struct channel *ch);

	void updateChannels(char side);

	/* clear
	 * delete all channels and groups */

	void clear();

	/* setChannelWithActions
	 * add 'R' button if channel has actions, and set recorder to active. */

	void setChannelWithActions(struct channel *ch);

	bool hasScrollbar();

	Fl_Group *gChannelsL;
	Fl_Group *gChannelsR;
};

#endif
