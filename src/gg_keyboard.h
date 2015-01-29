/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gg_keyboard
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


#ifndef GG_KEYBOARD_H
#define GG_KEYBOARD_H


#include <FL/Fl.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Group.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_Button.H>
#include "utils.h"


class gColumn : public Fl_Group
{
private:

	static void cb_addChannel  (Fl_Widget *v, void *p);
	inline void __cb_addChannel();

	int openTypeMenu();

	class gClick      *addChannelBtn;
	class gResizerBar *resizer;
	class gKeyboard   *parent;

	int index;

public:

	gColumn(int x, int y, int w, int h, int index, class gKeyboard *parent);
	~gColumn();

	/* addChannel
	 * add a new channel in this column and set the internal pointer
	 * to channel to 'ch'. */

	class gChannel *addChannel(class Channel *ch);

	/* handle */

	int handle(int e);

  /* resize
   * custom resize behavior. */

  void resize(int x, int y, int w, int h);

	/* deleteChannel
	 * remove the channel 'gch' from this column. */

	void deleteChannel(gChannel *gch);

	/* refreshChannels
	 * update channels' graphical statues. Called on each GUI cycle. */

	void refreshChannels();

	/* clear
	 * remove all channels from the column. If full==true, delete also the
	 * "add new channel" button. This method ovverrides the inherited one
	 * from Fl_Group. */

	void clear(bool full=false);

	void draw();

	inline int  getIndex()      { return index; }
	inline void setIndex(int i) { index = i; }
	inline bool isEmpty()       { return children() == 1; }
};


/* -------------------------------------------------------------------------- */


class gKeyboard : public Fl_Scroll
{
private:

	static void cb_addColumn  (Fl_Widget *v, void *p);
	inline void __cb_addColumn();

	bool bckspcPressed;
	bool endPressed;
	bool spacePressed;
	bool enterPressed;

	/* indexColumn
	 * the last index used for column. */

	static int indexColumn;

	class gClick *addColumnBtn;

	/* columns
	 * a vector of columns which in turn contain channels. */

	gVector<gColumn*> columns;

public:

	gKeyboard(int X, int Y, int W, int H);

	int handle(int e);

	/* init
	 * build the initial setup of empty channels. */

	void init();

	/* addChannel
	 * add a new channel to gChannels. Used by callbacks and during
	 * patch loading. Requires Channel (and not gChannel). If build is
	 * set to true, also generate the corresponding column.*/

	gChannel *addChannel(int column, class Channel *ch, bool build=false);

	/* deleteChannel
	 * delete a channel from gChannels<> where gChannel->ch == ch and remove
	 * it from the stack. */

	void deleteChannel(gChannel *gch);

	/* freeChannel
	 * free a channel from gChannels<> where gChannel->ch == ch. No channels
	 * are deleted */

	void freeChannel(gChannel *gch);

	/* updateChannel
	 * wrapper function to call gch->update(). */

	void updateChannel(gChannel *gch);

	/* organizeColumns
	 * reorganize columns layout by removing empty gaps. */

	void organizeColumns();

	/* refreshColumns
	 * refresh each column's channel, called on each GUI cycle. */

	void refreshColumns();

	/* getColumn
	 * return the column with index 'index', or NULL if not found. */

	gColumn *getColumn(int index);

	/* clear
	 * delete all channels and groups. */

	void clear();

	/* setChannelWithActions
	 * add 'R' button if channel has actions, and set recorder to active. */

	void setChannelWithActions(class gSampleChannel *gch);

	/* printChannelMessage
	 * given any output by glue_loadChannel, print the message on screen
	 * on a gdAlert subwindow. */

	void printChannelMessage(int res);
};


#endif
