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


#ifndef GE_SAMPLE_CHANNEL_H
#define GE_SAMPLE_CHANNEL_H


#include "channel.h"


class geStatusButton;


namespace giada {
namespace m 
{ 
class SampleChannel; 
}
namespace v
{
class geChannelMode;

class geSampleChannel : public geChannel
{
public:

	geSampleChannel(int x, int y, int w, int h, ID channelId);

	void resize(int x, int y, int w, int h) override;

	void refresh() override;
	void changeSize(int h) override;

	geChannelMode* modeBox;
	geButton*      readActions;

private:

	static void cb_playButton(Fl_Widget* v, void* p);
	static void cb_openMenu(Fl_Widget* v, void* p);
	static void cb_readActions(Fl_Widget* v, void* p);
	void cb_playButton();
	void cb_openMenu();
	void cb_readActions();
};
}} // giada::v::


#endif
