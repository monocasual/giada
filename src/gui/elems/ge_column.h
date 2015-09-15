/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_column
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


#ifndef GE_COLUMN_H
#define GE_COLUMN_H


#include <FL/Fl.H>
#include <FL/Fl_Group.H>


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
  inline int  countChannels() { return children(); }
};


#endif
