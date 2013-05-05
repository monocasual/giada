/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_mixed
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013 Giovanni A. Zuliani | Monocasual
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
 * ------------------------------------------------------------------ */


#ifndef GE_MIXED_H
#define GE_MIXED_H

#include <stdio.h>
#include <dirent.h>
#include <stdint.h>  // for intptr_t
#include <string>
#include <FL/Fl.H>
#include <FL/Fl_Menu_Window.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Repeat_Button.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Box.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Dial.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_Menu_Button.H>
#include <FL/Fl_Hold_Browser.H>
#include <FL/Fl_Radio_Button.H>
#include <FL/Fl_Progress.H>
#include <FL/Fl_Input.H>
#include <FL/Fl_Int_Input.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Scroll.H>

#ifdef _WIN32
	#include <shlobj.h>  // for SHGetFolderPath
#endif


/* cb_window_closer
 * callback for when closing windows. Deletes the widget (delete). */

void __cb_window_closer(Fl_Widget *v, void *p);


/* ------------------------------------------------------------------ */


/* gClick
 * a normal button. */

class gClick : public Fl_Button {
public:
	gClick(int x, int y, int w, int h, const char *L=0, const char **imgOff=NULL, const char **imgOn=NULL);
	void draw();
	const char **imgOff;
	const char **imgOn;
	Fl_Color bgColor0;   // background not clicked
	Fl_Color bgColor1;   // background clicked
	Fl_Color bdColor;    // border
	Fl_Color txtColor;	 // testo
};


/* ------------------------------------------------------------------ */


class gClickRepeat : public Fl_Repeat_Button {
public:
	gClickRepeat(int x, int y, int w, int h, const char *L=0, const char **imgOff=NULL, const char **imgOn=NULL);
	void draw();
	const char **imgOff;
	const char **imgOn;
};


/* ------------------------------------------------------------------ */


/* gButton
 * exactly as gClick but with a unique id inside of it. Used for the
 * channels */

class gButton : public gClick {
public:
	gButton(int X,int Y,int W,int H,const char *L=0, const char **imgOff=NULL, const char **imgOn=NULL);
	int key;
	int id;
};


/* ------------------------------------------------------------------ */


class gInput : public Fl_Input {
public:
	gInput(int x, int y, int w, int h, const char *L=0);
};


/* ------------------------------------------------------------------ */


class gDial : public Fl_Dial {
public:
	gDial(int x, int y, int w, int h, const char *L=0);
	void draw();
};


/* ------------------------------------------------------------------ */


class gBox : public Fl_Box {
public:
	gBox(int x, int y, int w, int h, const char *L=0, Fl_Align al=FL_ALIGN_CENTER);
};


/* ------------------------------------------------------------------ */


class gCheck : public Fl_Check_Button {
public:
	gCheck(int x, int y, int w, int h, const char *L=0);
	void draw();
};


/* ------------------------------------------------------------------ */


class gRadio : public Fl_Radio_Button {
public:
	gRadio(int x, int y, int w, int h, const char *L=0);
	void draw();
};


/* ------------------------------------------------------------------ */


class gProgress : public Fl_Progress {
public:
	gProgress(int x, int y, int w, int h, const char *L=0);
};


/* ------------------------------------------------------------------ */


class gSoundMeter : public Fl_Box {
public:
	gSoundMeter(int X,int Y,int W,int H,const char *L=0);
	void draw();
	bool clip;
	float mixerPeak;	// peak from mixer
private:
	float peak;
	float peak_old;
	float db_level;
	float db_level_old;
};


/* ------------------------------------------------------------------ */


class gBeatMeter : public Fl_Box {
public:
	gBeatMeter(int X,int Y,int W,int H,const char *L=0);
	void draw();
};


/* ------------------------------------------------------------------ */


class gModeBox : public Fl_Menu_Button {
private:
	static void cb_change_chanmode(Fl_Widget *v, void *p);
	inline void __cb_change_chanmode(int mode);

	struct channel *ch;

public:
	gModeBox(int x, int y, int w, int h, struct channel *ch, const char *l=0);
	void draw();
	//int id;
	//static int id_generator;
};


/* ------------------------------------------------------------------ */


class gChoice : public Fl_Choice {
public:
	gChoice(int X,int Y,int W,int H,const char *L=0, bool angle=true);
	void draw();

	inline void show(const char *c) {value(find_index(c)); }

	bool angle;
	int  id;
};


/* ------------------------------------------------------------------ */


/* Status
 * The chan status. */

class gStatus : public Fl_Box {
public:
	gStatus(int X, int Y, int W, int H, struct channel *ch, const char *L=0);
	void draw();
	struct channel *ch;
};


/* ------------------------------------------------------------------ */


/* gDrawBox
 * custom boxes in FLTK. */

#define G_BOX FL_FREE_BOXTYPE
void gDrawBox(int x, int y, int w, int h, Fl_Color c);


/* ------------------------------------------------------------------ */


/* gLiquidScroll
 * custom scroll that tells children to follow scroll's width when
 * resized. Thanks to Greg Ercolano from FLTK dev team.
 * http://seriss.com/people/erco/fltk/ */

class gLiquidScroll : public Fl_Scroll {
public:
	gLiquidScroll(int x, int y, int w, int h, const char *l=0);
	void resize(int x, int y, int w, int h);
};


/* ------------------------------------------------------------------ */


class gSlider : public Fl_Slider {
public:
	gSlider(int x, int y, int w, int h, const char *l=0);
	int id;
};

#endif
