/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_sampleChannel
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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
#include "channelButton.h"


class geSampleChannel : public geChannel
{
private:

	static void cb_button        (Fl_Widget *v, void *p);
	static void cb_openMenu      (Fl_Widget *v, void *p);
	static void cb_readActions   (Fl_Widget *v, void *p);

	inline void __cb_button      ();
	inline void __cb_openMenu    ();
	inline void __cb_readActions ();

public:

	geSampleChannel(int x, int y, int w, int h, class SampleChannel *ch);

	void reset   ();
	void update  ();
	void refresh ();
	void resize  (int x, int y, int w, int h);

	/* show/hideActionButton
	Adds or removes 'R' button when actions are available. */

	void showActionButton();
	void hideActionButton();

	class geChannelMode *modeBox;
	class gClick 	 *readActions;
};


/* -------------------------------------------------------------------------- */


class geSampleChannelButton : public geChannelButton
{
public:
	geSampleChannelButton(int x, int y, int w, int h, const char *l=0);
	int handle(int e);
};


#endif
