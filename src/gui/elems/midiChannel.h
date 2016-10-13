/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_midiChannel
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


#ifndef GE_MIDI_CHANNEL_H
#define GE_MIDI_CHANNEL_H


#include "channel.h"
#include "channelButton.h"


class geMidiChannel : public geChannel
{
private:

	static void cb_button        (Fl_Widget *v, void *p);
	static void cb_mute          (Fl_Widget *v, void *p);
	static void cb_solo          (Fl_Widget *v, void *p);
	static void cb_openMenu      (Fl_Widget *v, void *p);
	static void cb_changeVol     (Fl_Widget *v, void *p);
#ifdef WITH_VST
	static void cb_openFxWindow  (Fl_Widget *v, void *p);
#endif

	inline void __cb_mute        ();
	inline void __cb_solo        ();
	inline void __cb_changeVol   ();
	inline void __cb_button      ();
	inline void __cb_openMenu    ();
	inline void __cb_readActions ();
#ifdef WITH_VST
	inline void __cb_openFxWindow();
#endif

	void breakAtArm();
	void breakAtFx(int delta=(BREAK_DELTA/* + (BREAK_UNIT*2)*/));
	void breakAtNone();

public:

	geMidiChannel(int x, int y, int w, int h,  class MidiChannel *ch);

	void reset   ();
	void update  ();
	void refresh ();
	int  keyPress(int event);
	void resize  (int x, int y, int w, int h);

	class MidiChannel *ch;
};


/* -------------------------------------------------------------------------- */


class geMidiChannelButton : public geChannelButton
{
public:
	geMidiChannelButton(int x, int y, int w, int h, const char *l=0);
	int handle(int e);
};


#endif
