/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 * gd_mainWindow
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2014 Giovanni A. Zuliani | Monocasual
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


#ifndef GD_MAINWINDOW_H
#define GD_MAINWINDOW_H


#include <FL/Fl.H>
#include <FL/x.H>
#include "ge_mixed.h"
#include "ge_window.h"


/* ------------------------------------------------------------------ */


class gdMainWindow : public gWindow
{
private:

	static void cb_endprogram  (Fl_Widget *v, void *p);
	inline void __cb_endprogram();

public:

	class gKeyboard   *keyboard;
	class gSongMeter  *songMeter;
	class gBeatMeter  *beatMeter;
	class gMenu       *menu;
	class gInOut      *inOut;
	class gController *controller;
	class gTiming     *timing;

	gdMainWindow(int w, int h, const char *title, int argc, char **argv);
};


/* ------------------------------------------------------------------ */


class gInOut : public Fl_Group
{
private:

	class gSoundMeter *outMeter;
	class gSoundMeter *inMeter;
	class gBeatMeter  *beatMeter;
	class gDial				*outVol;
	class gDial				*inVol;
#ifdef WITH_VST
	class gButton			*masterFxOut;
	class gButton			*masterFxIn;
	class gClick      *inToOut;
#endif

	static void cb_outVol     (Fl_Widget *v, void *p);
	static void cb_inVol      (Fl_Widget *v, void *p);
#ifdef WITH_VST
	static void cb_masterFxOut(Fl_Widget *v, void *p);
	static void cb_masterFxIn (Fl_Widget *v, void *p);
	static void cb_inToOut    (Fl_Widget *v, void *p);
#endif

	inline void __cb_outVol     ();
	inline void __cb_inVol      ();
#ifdef WITH_VST
	inline void __cb_masterFxOut();
	inline void __cb_masterFxIn ();
	inline void __cb_inToOut    ();
#endif

public:

	gInOut(int x, int y);
	
	void refresh();

	inline void setOutVol(float v) { outVol->value(v); }
	inline void setInVol (float v) { inVol->value(v); }
};


/* ------------------------------------------------------------------ */


class gMenu : public Fl_Group
{
private:

	class gClick *file;
	class gClick *edit;
	class	gClick *config;
	class gClick *about;

	static void cb_about (Fl_Widget *v, void *p);
	static void cb_config(Fl_Widget *v, void *p);
	static void cb_file  (Fl_Widget *v, void *p);
	static void cb_edit  (Fl_Widget *v, void *p);

	inline void __cb_about ();
	inline void __cb_config();
	inline void __cb_file  ();
	inline void __cb_edit  ();

public:

	gMenu(int x, int y);
};


/* ------------------------------------------------------------------ */


class gController : public Fl_Group
{
private:

	class gClick *rewind;
	class gClick *play;
	class gClick *recAction;
	class gClick *recInput;
	class gClick *metronome;

	static void cb_rewind   (Fl_Widget *v, void *p);
	static void cb_play     (Fl_Widget *v, void *p);
	static void cb_recAction(Fl_Widget *v, void *p);
	static void cb_recInput (Fl_Widget *v, void *p);
	static void cb_metronome(Fl_Widget *v, void *p);

	inline void __cb_rewind   ();
	inline void __cb_play     ();
	inline void __cb_recAction();
	inline void __cb_recInput ();
	inline void __cb_metronome();

public:

	gController(int x, int y);

	void updatePlay     (int v);
	void updateMetronome(int v);
	void updateRecInput (int v);
	void updateRecAction(int v);
};


/* ------------------------------------------------------------------ */


class gTiming : public Fl_Group
{
private:

	class gClick   *bpm;
	class gClick   *meter;
	class gChoice  *quantizer;
	class gClick   *multiplier;
	class gClick   *divider;
	class gClick   *songmultiplier;
	class gClick   *songdivider;
	class gClick   *advance;
	class gCheck   *suspend;

	static void cb_bpm       (Fl_Widget *v, void *p);
	static void cb_meter     (Fl_Widget *v, void *p);
	static void cb_quantizer (Fl_Widget *v, void *p);
	static void cb_multiplier(Fl_Widget *v, void *p);
	static void cb_divider   (Fl_Widget *v, void *p);
	static void cb_songmultiplier(Fl_Widget *v, void *p);
	static void cb_songdivider   (Fl_Widget *v, void *p);
	static void cb_advance   (Fl_Widget *v, void *p);
	static void cb_suspend   (Fl_Widget *v, void *p);

	inline void __cb_bpm();
	inline void __cb_meter();
	inline void __cb_quantizer();
	inline void __cb_multiplier();
	inline void __cb_divider();
	inline void __cb_songmultiplier();
	inline void __cb_songdivider();
	inline void __cb_advance();
	inline void __cb_suspend();

public:

	gTiming(int x, int y);
	
	void setBpm(const char *v);
	void setMeter(int beats, int bars);
};


#endif
