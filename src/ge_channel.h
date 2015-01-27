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


class gChannel : public Fl_Group
{
public:

	gChannel(int x, int y, int w, int h);

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

	class gButton *button;
	class gStatus *status;
	class gClick  *sampleButton;
	class gDial   *vol;
	class gClick 	*mute;
	class gClick 	*solo;
#ifdef WITH_VST
	class gButton *fx;
#endif

};


/* -------------------------------------------------------------------------- */


class gSampleChannel : public gChannel
{
private:

	static void cb_button        (Fl_Widget *v, void *p);
	static void cb_mute          (Fl_Widget *v, void *p);
	static void cb_solo          (Fl_Widget *v, void *p);
	static void cb_openMenu      (Fl_Widget *v, void *p);
	static void cb_changeVol     (Fl_Widget *v, void *p);
	static void cb_readActions   (Fl_Widget *v, void *p);
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

	void openBrowser(int type);

public:

	gSampleChannel(int x, int y, int w, int h, class SampleChannel *ch);

	void reset   ();
	void update  ();
	void refresh ();
	int  keyPress(int event);
	void resize  (int x, int y, int w, int h);

	/* add/delActionButton
	 * add or remove 'R' button when actions are available. 'Status' is
	 * the initial status of the button: on or off.
	 * If force==true remove the button with no further checks. */

	void addActionButton();
	void delActionButton(bool force=false);

	class gModeBox *modeBox;
	class gClick 	 *readActions;

	class SampleChannel *ch;
};


/* -------------------------------------------------------------------------- */


class gMidiChannel : public gChannel
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

public:

	gMidiChannel(int x, int y, int w, int h,  class MidiChannel *ch);

	void reset   ();
	void update  ();
	void refresh ();
	int  keyPress(int event);
	void resize  (int x, int y, int w, int h);

	class MidiChannel *ch;
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


class gModeBox : public Fl_Menu_Button {
private:
	static void cb_change_chanmode(Fl_Widget *v, void *p);
	inline void __cb_change_chanmode(int mode);

	class SampleChannel *ch;

public:
	gModeBox(int x, int y, int w, int h, class SampleChannel *ch, const char *l=0);
	void draw();
};

#endif
