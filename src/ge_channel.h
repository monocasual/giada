/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_channel
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


#ifndef GE_CHANNEL_H
#define GE_CHANNEL_H


#include <FL/Fl_Scroll.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Menu_Button.H>
#include "ge_mixed.h"


class gChannel : public Fl_Group
{
protected:

	/* define some breakpoints for dynamic resize */

#ifdef WITH_VST
	static const int BREAK_READ_ACTIONS = 212;
	static const int BREAK_MODE_BOX     = 188;
	static const int BREAK_FX           = 164;
	static const int BREAK_DELTA        = 120;
#else
	static const int BREAK_READ_ACTIONS = 188;
	static const int BREAK_MODE_BOX     = 164;
	static const int BREAK_FX           = 140;
	static const int BREAK_DELTA        = 96;
#endif
	static const int BREAK_UNIT         = 24;

public:

	gChannel(int x, int y, int w, int h, int type);

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

#ifdef WITH_VST

	/* lightUpFx
	 * light up the FX button if the channel has some fx's loaded. */

	void lightUpFx();
#endif

	class gButton *button;
	class gStatus *status;
	class gMainButton *mainButton;
	class gDial   *vol;
	class gClick 	*mute;
	class gClick 	*solo;
#ifdef WITH_VST
	class gFxButton *fx;
#endif

	int type;
};


/* -------------------------------------------------------------------------- */


class gStatus : public Fl_Box
{
public:
	gStatus(int X, int Y, int W, int H, class SampleChannel *ch, const char *L=0);
	void draw();
	class SampleChannel *ch;
};


/* -------------------------------------------------------------------------- */


class gModeBox : public Fl_Menu_Button
{
private:
	static void cb_change_chanmode(Fl_Widget *v, void *p);
	inline void __cb_change_chanmode(int mode);

	class SampleChannel *ch;

public:
	gModeBox(int x, int y, int w, int h, class SampleChannel *ch, const char *l=0);
	void draw();
};


/* -------------------------------------------------------------------------- */


/* gMainButton
 * base main button for MIDI and Sample Channels. */

class gMainButton : public gClick
{
public:
	gMainButton(int x, int y, int w, int h, const char *l=0);
	virtual int handle(int e) = 0;
};


#endif
