/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_channel
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


#ifndef GE_CHANNEL_H
#define GE_CHANNEL_H


#include <FL/Fl_Group.H>


class geChannel : public Fl_Group
{
protected:

	/* Define some breakpoints for dynamic resize. BREAK_DELTA: base amount of
	pixels to shrink sampleButton. */

#ifdef WITH_VST
	static const int BREAK_READ_ACTIONS = 240;
	static const int BREAK_MODE_BOX     = 216;
	static const int BREAK_FX           = 192;
	static const int BREAK_ARM          = 168;
#else
	static const int BREAK_READ_ACTIONS = 216;
	static const int BREAK_MODE_BOX     = 192;
	static const int BREAK_ARM          = 168;
#endif

	/* blink
	 * blink button when channel is in wait/ending status. */

	void blink();

	/* setColorByStatus
	 * update colors depending on channel status. */

	void setColorsByStatus(int playStatus, int recStatus);

	/* handleKey
	 * method wrapped by virtual handle(int e). */

	int handleKey(int e, int key);

	/* packWidgets
	Spread widgets across available space. */

	void packWidgets();

public:

	geChannel(int x, int y, int w, int h, int type);

	/* reset
	 * reset channel to initial status. */

	virtual void reset() = 0;

	/* update
	 * update the label of sample button and everything else such as 'R'
	 * button, key box and so on, according to global values. */

	virtual void update() = 0;

	/* refresh
	 * update graphics. */

	virtual void refresh() = 0;

	/* keypress
	 * what to do when the corresponding key is pressed. */

	virtual int keyPress(int event) = 0;

	/* getColumnIndex
	 * return the numeric index of the column in which this channel is
	 * located. */

	int getColumnIndex();

	class gButton         *button;
	class gStatus         *status;
	class gClick 	        *arm;
	class geChannelButton *mainButton;
	class gClick 	        *mute;
	class gClick 	        *solo;
	class gDial           *vol;
#ifdef WITH_VST
	class gFxButton       *fx;
#endif

	int type;
};


#endif
