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
#include <FL/Fl_Double_Window.H>
#include <stdint.h>         // for intptr_t
#include "gd_bpmInput.h"
#include "gd_beatsInput.h"
#include "gd_actionEditor.h"
#include "gd_config.h"
#include "gg_keyboard.h"
#include "ge_mixed.h"
#include "gd_about.h"

#ifdef WITH_VST
#include "gd_pluginList.h"
#endif

#include "patch.h"
#include "conf.h"

#ifdef __linux__
	#include <sys/stat.h>			// for mkdir
#endif


class gdMainWindow : public gWindow {

private:

	static void cb_endprogram      (Fl_Widget *v, void *p);
	static void cb_change_bpm      (Fl_Widget *v, void *p);
	static void cb_change_batt     (Fl_Widget *v, void *p);
	static void cb_open_about_win  (Fl_Widget *v, void *p);
	static void cb_rewind_tracker  (Fl_Widget *v, void *p);
	static void cb_open_config_win (Fl_Widget *v, void *p);
	static void cb_startstop       (Fl_Widget *v, void *p);
	static void cb_rec             (Fl_Widget *v, void *p);
	static void cb_inputRec        (Fl_Widget *v, void *p);
	static void cb_quantize        (Fl_Widget *v, void *p);
	static void cb_outVol          (Fl_Widget *v, void *p);
	static void cb_inVol           (Fl_Widget *v, void *p);
	static void cb_open_file_menu  (Fl_Widget *v, void *p);
	static void cb_open_edit_menu  (Fl_Widget *v, void *p);
	static void cb_metronome       (Fl_Widget *v, void *p);
	static void cb_beatsMultiply   (Fl_Widget *v, void *p);
	static void cb_beatsDivide     (Fl_Widget *v, void *p);
#ifdef WITH_VST
	static void cb_openMasterFxOut (Fl_Widget *v, void *p);
	static void cb_openMasterFxIn  (Fl_Widget *v, void *p);
	static void cb_inToOut         (Fl_Widget *v, void *p);
#endif
	inline void __cb_endprogram();
	inline void __cb_change_bpm();
	inline void __cb_change_batt();
	inline void __cb_open_about_win();
	inline void __cb_rewind_tracker();
	inline void __cb_open_loadpatch_win();
	inline void __cb_open_savepatch_win();
	inline void __cb_open_saveproject_win();
	inline void __cb_open_config_win();
	inline void __cb_startstop();
	inline void __cb_rec();
	inline void __cb_inputRec();
	inline void __cb_quantize(int v);
	inline void __cb_outVol();
	inline void __cb_inVol();
	inline void __cb_open_file_menu();
	inline void __cb_open_edit_menu();
	inline void __cb_metronome();
	inline void __cb_beatsMultiply();
	inline void __cb_beatsDivide();
#ifdef WITH_VST
	inline void __cb_openMasterFxOut();
	inline void __cb_openMasterFxIn();
	inline void __cb_inToOut();
#endif

public:

	class gClick      *bpm;
	class gClick      *beats;
	class gClick      *beats_mul;
	class gClick      *beats_div;
	class Keyboard    *keyboard;		// Fl_Group
	class gSoundMeter *outMeter;
	class gSoundMeter *inMeter;
	class gBeatMeter  *beatMeter;
	class gDial				*outVol;
	class gDial				*inVol;
	class gClick      *beat_rew;
	class gClick      *beat_stop;
	class gClick      *beat_rec;
	class gClick      *input_rec;
	class gClick      *menu_file;
	class gClick      *menu_edit;
	class	gClick      *menu_config;
	class gClick      *menu_about;
	class gChoice     *quantize;
	class gClick      *metronome;
#ifdef WITH_VST
	class gButton			*masterFxOut;
	class gButton			*masterFxIn;
	class gClick      *inToOut;
#endif

	gdMainWindow(int x, int y, int w, int h, const char *title, int argc, char **argv);
	~gdMainWindow();
};


/* ------------------------------------------------------------------ */


class gInOut : public Fl_Group {

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

	static void cb_outVol         (Fl_Widget *v, void *p);
	static void cb_inVol          (Fl_Widget *v, void *p);
#ifdef WITH_VST
	static void cb_openMasterFxOut(Fl_Widget *v, void *p);
	static void cb_openMasterFxIn (Fl_Widget *v, void *p);
	static void cb_inToOut        (Fl_Widget *v, void *p);
#endif

	inline void __cb_outVol         ();
	inline void __cb_inVol          ();
#ifdef WITH_VST
	inline void __cb_openMasterFxOut();
	inline void __cb_openMasterFxIn ();
	inline void __cb_inToOut        ();
#endif

	gInOut(int x, int y);
};

#endif
