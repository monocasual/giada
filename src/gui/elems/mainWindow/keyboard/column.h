/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#include <functional>
#include <FL/Fl_Pack.H>
#include "core/types.h"


class geButton;
class geResizerBar;


namespace giada {
namespace v
{
class geKeyboard;
class geChannel;
class geColumn : public Fl_Pack
{
public:

	geColumn(int x, int y, int w, int h, ID id);

	geChannel* getChannel(ID chanID) const;
	
	/* addChannel
	Adds a new channel in this column. */

	geChannel* addChannel(ID channelId, ChannelType t, int size);

	/* refreshChannels
	Updates channels' graphical statues. Called on each GUI cycle. */

	void refresh();

	void init(); 

	void forEachChannel(std::function<void(geChannel* c)> f) const;
	
	ID id;

private:

	static void cb_addChannel(Fl_Widget* v, void* p);
	void cb_addChannel();

	geButton* m_addChannelBtn;
};
}} // giada::v::


#endif
