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


#ifndef GE_COLUMN_H
#define GE_COLUMN_H


#include <FL/Fl_Group.H>


class Channel;
class geButton;
class geChannel;
class geResizerBar;
class geKeyboard;


class geColumn : public Fl_Group
{
private:

	static void cb_addChannel  (Fl_Widget* v, void* p);
	inline void __cb_addChannel();

	int openTypeMenu();

	geButton*     m_addChannelBtn;
	geResizerBar* m_resizer;
	geKeyboard*   m_parent;

	int m_index;

public:

	geColumn(int x, int y, int w, int h, int index, geKeyboard* parent);
	~geColumn();

	/* addChannel
	Adds a new channel in this column and set the internal pointer to channel 
	to 'ch'. */

	geChannel* addChannel(Channel* ch, int size);

	int handle(int e) override;
	void draw() override;
  void resize(int x, int y, int w, int h) override;

	/* clear
	Removes all channels from the column. If full==true, delete also the "add new 
	channel" button. */

	void clear(bool full=false);

	/* deleteChannel
	Removes the channel 'gch' from this column. */

	void deleteChannel(geChannel* gch);

	void repositionChannels();

	/* refreshChannels
	Updates channels' graphical statues. Called on each GUI cycle. */

	void refreshChannels();

	Channel* getChannel(int i);
	int getIndex();
	void setIndex(int i);
	bool isEmpty();   
  int countChannels();
};


#endif
