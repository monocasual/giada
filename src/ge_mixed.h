/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_mixed
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


/* -------------------------------------------------------------------------- */


class gBaseButton : public Fl_Button
{
private:
	std::string initLabel;

	void trimLabel();

public:
  gBaseButton(int x, int y, int w, int h, const char *l=0);
  void resize(int x, int y, int w, int h);
	void label(const char *l);
	const char *label();
};


/* -------------------------------------------------------------------------- */


/* gClick
 * a normal button. */

class gClick : public gBaseButton
{
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


/* -------------------------------------------------------------------------- */


class gClickRepeat : public Fl_Repeat_Button
{
public:
	gClickRepeat(int x, int y, int w, int h, const char *L=0, const char **imgOff=NULL, const char **imgOn=NULL);
	void draw();
	const char **imgOff;
	const char **imgOn;
};


/* -------------------------------------------------------------------------- */


/* gButton
 * exactly as gClick but with a unique id inside of it. Used for the buttons in
 * channels and for FXs. */

class gButton : public gClick
{
public:
	gButton(int X,int Y,int W,int H,const char *L=0, const char **imgOff=NULL, const char **imgOn=NULL);
	int key;
	int id;
};


/* -------------------------------------------------------------------------- */


class gInput : public Fl_Input
{
public:
	gInput(int x, int y, int w, int h, const char *L=0);
};


/* -------------------------------------------------------------------------- */


class gDial : public Fl_Dial
{
public:
	gDial(int x, int y, int w, int h, const char *L=0);
	void draw();
};


/* -------------------------------------------------------------------------- */


class gBox : public Fl_Box
{
public:
	gBox(int x, int y, int w, int h, const char *L=0, Fl_Align al=FL_ALIGN_CENTER);
};


/* -------------------------------------------------------------------------- */


class gCheck : public Fl_Check_Button
{
public:
	gCheck(int x, int y, int w, int h, const char *L=0);
	void draw();
};


/* -------------------------------------------------------------------------- */


class gRadio : public Fl_Radio_Button
{
public:
	gRadio(int x, int y, int w, int h, const char *L=0);
	void draw();
};


/* -------------------------------------------------------------------------- */


class gProgress : public Fl_Progress
{
public:
	gProgress(int x, int y, int w, int h, const char *L=0);
};


/* -------------------------------------------------------------------------- */


class gSoundMeter : public Fl_Box
{
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


/* -------------------------------------------------------------------------- */


class gBeatMeter : public Fl_Box
{
public:
	gBeatMeter(int X,int Y,int W,int H,const char *L=0);
	void draw();
};


/* -------------------------------------------------------------------------- */


class gChoice : public Fl_Choice
{
public:

	gChoice(int X,int Y,int W,int H,const char *L=0, bool angle=true);
	void draw();

	inline void show(const char *c) {value(find_index(c)); }

	bool angle;
	int  id;
};


/* -------------------------------------------------------------------------- */


/* gDrawBox
 * custom boxes in FLTK. */

#define G_BOX FL_FREE_BOXTYPE
void gDrawBox(int x, int y, int w, int h, Fl_Color c);


/* -------------------------------------------------------------------------- */


/* gLiquidScroll
 * custom scroll that tells children to follow scroll's width when
 * resized. Thanks to Greg Ercolano from FLTK dev team.
 * http://seriss.com/people/erco/fltk/ */

class gLiquidScroll : public Fl_Scroll
{
public:
	gLiquidScroll(int x, int y, int w, int h, const char *l=0);
	void resize(int x, int y, int w, int h);
};


/* -------------------------------------------------------------------------- */


/* gScroll
 * custom scroll with nice scrollbars and something else. */

class gScroll : public Fl_Scroll
{
public:
	gScroll(int x, int y, int w, int h, int type=Fl_Scroll::BOTH);
};


/* -------------------------------------------------------------------------- */

/* gResizerBar
 * 'resizer bar' between widgets Fl_Scroll. Thanks to Greg Ercolano from
 * FLTK dev team. http://seriss.com/people/erco/fltk/
 *
 * Shows a resize cursor when hovered over.
 * Assumes:
 *     - Parent is an Fl_Scroll
 *     - All children of Fl_Scroll are vertically arranged
 *     - The widget above us has a bottom edge touching our top edge
 *       ie. (w->y()+w->h() == this->y())
 *
 * When this widget is dragged:
 *     - The widget above us (with a common edge) will be /resized/
 *       vertically
 *     - All children below us will be /moved/ vertically */

/* TODO - use more general variable names
 * (last_y -> last_?, min_h -> min_?, ...) */

class gResizerBar : public Fl_Box
{
private:
  bool vertical;
	int  orig_h;
	int  last_y;
	int  min_h;   // min height for widget above us

	void HandleDrag(int diff);

public:

  /* 'vertical' defines the bar movement. Vertical=true: the bar moves
   * vertically (up and down). */

	gResizerBar(int x, int y, int w, int h, bool vertical=true);
	//~gResizerBar();

  inline void setMinSize(int val) { min_h = val; }
  inline int  getMinSize()        { return min_h; }

  int  handle(int e);
  void resize(int x, int y, int w, int h);
};


/* -------------------------------------------------------------------------- */


class gSlider : public Fl_Slider
{
public:
	gSlider(int x, int y, int w, int h, const char *l=0);
	int id;
};


/* -------------------------------------------------------------------------- */


/* gFxButton
 * a simple gClick with 'full' parameter (i.e. has plugins). If 'full' is true,
 * draw something somewhere. */

class gFxButton : public gClick
{
public:
	gFxButton(int x, int y, int w, int h, const char **imgOff=NULL, const char **imgOn=NULL);
	void draw();
	bool full;
};


#endif
