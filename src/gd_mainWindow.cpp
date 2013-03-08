/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_mainWindow
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


#include "gd_mainWindow.h"
#include "graphics.h"
#include "gd_warnings.h"
#include "glue.h"
#include "mixer.h"
#include "recorder.h"
#include "gd_browser.h"
#include "mixerHandler.h"
#include "pluginHost.h"


extern Mixer	   		 G_Mixer;
extern Patch     		 G_Patch;
extern Conf	 	   		 G_Conf;
extern gdMainWindow *mainWin;
extern bool	 		 		 G_quit;
extern bool 		 		 G_audio_status;

#if defined(WITH_VST)
extern PluginHost  	 G_PluginHost;
#endif


gdMainWindow::gdMainWindow(int X, int Y, int W, int H, const char *title, int argc, char **argv)
: gWindow(X, Y, W, H, title) {

	Fl::visible_focus(0);
	Fl::background(25, 25, 25);
	Fl::set_boxtype(G_BOX, gDrawBox, 1, 1, 2, 2);    // custom box G_BOX

	menu_file 	= new gClick(8,   -1, 70, 21, "file");
	menu_edit	  = new gClick(82,  -1, 70, 21, "edit");
	menu_config	= new gClick(156, -1, 70, 21, "config");
	menu_about	= new gClick(230, -1, 70, 21, "about");

	quantize    = new gChoice(632, 49, 40, 15, "", false);
	bpm         = new gClick(676,  49, 40, 15);
	beats       = new gClick(724,  49, 40, 15, "4/1");
	beats_mul   = new gClick(768,  49, 15, 15, "×");
	beats_div   = new gClick(787,  49, 15, 15, "÷");

#if defined(WITH_VST)
	masterFxIn  = new gButton(408, 8, 20, 20, "", fxOff_xpm, fxOn_xpm);
	inVol		    = new gDial  (432, 8, 20, 20);
	inMeter     = new gSoundMeter(456, 13, 140, 10);
	inToOut     = new gClick (600, 13, 10, 10, "");
	outMeter    = new gSoundMeter(614, 13, 140, 10);
	outVol		  = new gDial  (758, 8, 20, 20);
	masterFxOut = new gButton(782, 8, 20, 20, "", fxOff_xpm, fxOn_xpm);
#else
	outMeter    = new gSoundMeter(638, 13, 140, 10);
	inMeter     = new gSoundMeter(494, 13, 140, 10);
	outVol		  = new gDial(782, 8, 20, 20);
	inVol		    = new gDial(470, 8, 20, 20);
#endif

	beatMeter   = new gBeatMeter(100, 83, 609, 20);
	keyboard    = new Keyboard(8, 122, w()-16, 370);
	//keyboard->box(FL_BORDER_BOX);

	beat_rew		= new gClick(8,  39, 25, 25, "", rewindOff_xpm, rewindOn_xpm);
	beat_stop		= new gClick(37, 39, 25, 25, "", play_xpm, pause_xpm);
	beat_rec		= new gClick(66, 39, 25, 25, "", recOff_xpm, recOn_xpm);
	input_rec		= new gClick(95, 39, 25, 25, "", inputRecOff_xpm, inputRecOn_xpm);

	metronome   = new gClick(124, 49, 15, 15, "", metronomeOff_xpm, metronomeOn_xpm);
	end();

	char buf_bpm[6]; snprintf(buf_bpm, 6, "%f", G_Mixer.bpm);
	bpm->copy_label(buf_bpm);
	bpm->callback(cb_change_bpm);
	beats->callback(cb_change_batt);

	menu_about->callback(cb_open_about_win);
	menu_file->callback(cb_open_file_menu);
	menu_edit->callback(cb_open_edit_menu);
	menu_config->callback(cb_open_config_win);

	outVol->callback(cb_outVol);
	outVol->value(G_Mixer.outVol);
	inVol->callback(cb_inVol);
	inVol->value(G_Mixer.inVol);
#ifdef WITH_VST
	masterFxOut->callback(cb_openMasterFxOut);
	masterFxIn->callback(cb_openMasterFxIn);
	inToOut->callback(cb_inToOut);
	inToOut->type(FL_TOGGLE_BUTTON);
#endif

	beat_rew->callback(cb_rewind_tracker);
	beat_stop->callback(cb_startstop);
	beat_stop->type(FL_TOGGLE_BUTTON);

	beat_rec->callback(cb_rec);
	beat_rec->type(FL_TOGGLE_BUTTON);
	input_rec->callback(cb_inputRec);
	input_rec->type(FL_TOGGLE_BUTTON);

	beats_mul->callback(cb_beatsMultiply);
	beats_div->callback(cb_beatsDivide);

	metronome->type(FL_TOGGLE_BUTTON);
	metronome->callback(cb_metronome);

	callback(cb_endprogram);

	quantize->add("off", 0, cb_quantize, (void*)0);
	quantize->add("1b",  0, cb_quantize, (void*)1);
	quantize->add("2b",  0, cb_quantize, (void*)2);
	quantize->add("3b",  0, cb_quantize, (void*)3);
	quantize->add("4b",  0, cb_quantize, (void*)4);
	quantize->add("6b",  0, cb_quantize, (void*)6);
	quantize->add("8b",  0, cb_quantize, (void*)8);
	quantize->value(0); //  "off" by default

	gu_setFavicon(this);

	show(argc, argv);
}


gdMainWindow::~gdMainWindow() {}


/* ------------------------------------------------------------------ */



void gdMainWindow::cb_endprogram     (Fl_Widget *v, void *p)    { mainWin->__cb_endprogram(); }
void gdMainWindow::cb_change_bpm     (Fl_Widget *v, void *p)    { mainWin->__cb_change_bpm(); }
void gdMainWindow::cb_change_batt    (Fl_Widget *v, void *p) 		{ mainWin->__cb_change_batt(); }
void gdMainWindow::cb_open_about_win (Fl_Widget *v, void *p) 		{ mainWin->__cb_open_about_win(); }
void gdMainWindow::cb_rewind_tracker (Fl_Widget *v, void *p) 		{ mainWin->__cb_rewind_tracker(); }
void gdMainWindow::cb_open_config_win(Fl_Widget *v, void *p) 		{ mainWin->__cb_open_config_win(); }
void gdMainWindow::cb_startstop      (Fl_Widget *v, void *p)  	{ mainWin->__cb_startstop(); }
void gdMainWindow::cb_rec            (Fl_Widget *v, void *p) 		{ mainWin->__cb_rec(); }
void gdMainWindow::cb_inputRec       (Fl_Widget *v, void *p) 		{ mainWin->__cb_inputRec(); }
void gdMainWindow::cb_quantize       (Fl_Widget *v, void *p)  	{ mainWin->__cb_quantize((intptr_t)p); }
void gdMainWindow::cb_outVol         (Fl_Widget *v, void *p)  	{ mainWin->__cb_outVol(); }
void gdMainWindow::cb_inVol          (Fl_Widget *v, void *p)  	{ mainWin->__cb_inVol(); }
void gdMainWindow::cb_open_file_menu (Fl_Widget *v, void *p)  	{ mainWin->__cb_open_file_menu(); }
void gdMainWindow::cb_open_edit_menu (Fl_Widget *v, void *p)  	{ mainWin->__cb_open_edit_menu(); }
void gdMainWindow::cb_metronome      (Fl_Widget *v, void *p)    { mainWin->__cb_metronome(); }
void gdMainWindow::cb_beatsMultiply  (Fl_Widget *v, void *p)    { mainWin->__cb_beatsMultiply(); }
void gdMainWindow::cb_beatsDivide    (Fl_Widget *v, void *p)    { mainWin->__cb_beatsDivide(); }
#ifdef WITH_VST
void gdMainWindow::cb_openMasterFxOut(Fl_Widget *v, void *p)    { mainWin->__cb_openMasterFxOut(); }
void gdMainWindow::cb_openMasterFxIn (Fl_Widget *v, void *p)    { mainWin->__cb_openMasterFxIn(); }
void gdMainWindow::cb_inToOut        (Fl_Widget *v, void *p)    { mainWin->__cb_inToOut(); }
#endif


/* ------------------------------------------------------------------ */


void gdMainWindow::__cb_endprogram() {

	if (!gdConfirmWin("Warning", "Quit Giada: are you sure?"))
		return;

	G_quit = true;

	puts("GUI closing...");

	if (!G_Conf.write())
		puts("Error while saving configuration file!");
	else
		puts("Saving configuration...");

	puts("Mixer cleanup...");

	/* if G_audio_status we close the kernelAudio FIRST, THEN the mixer.
	 * The opposite could cause random segfaults (even now with RtAudio?). */

	if (G_audio_status) {
		kernelAudio::closeDevice();
		G_Mixer.close();
	}

	puts("Recorder cleanup...");
	recorder::clearAll();

#ifdef WITH_VST
	puts("Plugin Host cleanup...");
	G_PluginHost.freeAllStacks();
#endif

	puts("Giada "VERSIONE" closed.");
	hide();

	delete this;
}


/* ------------------------------------------------------------------ */


void gdMainWindow::__cb_change_bpm() {
	gu_openSubWindow(mainWin, new gdBpmInput(), WID_BPM);
}


/* ------------------------------------------------------------------ */


void gdMainWindow::__cb_change_batt() {
	gu_openSubWindow(mainWin, new gdBeatsInput(), WID_BEATS);
}


/* ------------------------------------------------------------------ */


void gdMainWindow::__cb_open_about_win() {
	gu_openSubWindow(mainWin, new gdAbout(), WID_ABOUT);
}


/* ------------------------------------------------------------------ */


void gdMainWindow::__cb_open_loadpatch_win() {
	gWindow *childWin = new gdBrowser("Load Patch", G_Conf.patchPath, 0, BROWSER_LOAD_PATCH);
	gu_openSubWindow(mainWin, childWin, WID_FILE_BROWSER);
}


/* ------------------------------------------------------------------ */


void gdMainWindow::__cb_open_savepatch_win() {
	gWindow *childWin = new gdBrowser("Save Patch", G_Conf.patchPath, 0, BROWSER_SAVE_PATCH);
	gu_openSubWindow(mainWin, childWin, WID_FILE_BROWSER);
}


/* ------------------------------------------------------------------ */


void gdMainWindow::__cb_open_saveproject_win() {
	gWindow *childWin = new gdBrowser("Save Project", G_Conf.patchPath, 0, BROWSER_SAVE_PROJECT);
	gu_openSubWindow(mainWin, childWin, WID_FILE_BROWSER);
}


/* ------------------------------------------------------------------ */


void gdMainWindow::__cb_open_config_win() {
	gu_openSubWindow(mainWin, new gdConfig(380, 370), WID_CONFIG);
}


/* ------------------------------------------------------------------ */


void gdMainWindow::__cb_rewind_tracker() {
	mh_rewind();
}


/* ------------------------------------------------------------------ */


void gdMainWindow::__cb_startstop() {
	G_Mixer.running ? glue_stopSeq() : glue_startSeq();
}


/* ------------------------------------------------------------------ */


void gdMainWindow::__cb_rec() {
	recorder::active ? glue_stopRec() : glue_startRec();
}


/* ------------------------------------------------------------------ */


void gdMainWindow::__cb_inputRec() {
	if (G_Mixer.chanInput == -1) {
		if (!glue_startInputRec())
			gdAlert("No more channels available.");
	}
	else
		glue_stopInputRec();
}


/* ------------------------------------------------------------------ */


void gdMainWindow::__cb_quantize(int v) {
	glue_quantize(v);
}


/* ------------------------------------------------------------------ */


void gdMainWindow::__cb_outVol() {
	glue_setOutVol(outVol->value());
}


/* ------------------------------------------------------------------ */


void gdMainWindow::__cb_inVol() {
	glue_setInVol(inVol->value());
}


/* ------------------------------------------------------------------ */


void gdMainWindow::__cb_open_file_menu() {

	/* An Fl_Menu_Button is made of many Fl_Menu_Item */

	Fl_Menu_Item menu[] = {
		{"Open patch or project..."},
		{"Save patch..."},
		{"Save project..."},
		{"Quit Giada"},
		{0}
	};

	Fl_Menu_Button *b = new Fl_Menu_Button(0, 0, 100, 50);
	b->box(G_BOX);
	b->textsize(11);
	b->textcolor(COLOR_TEXT_0);
	b->color(COLOR_BG_0);

	const Fl_Menu_Item *m = menu->popup(Fl::event_x(),	Fl::event_y(), 0, 0, b);
	if (!m) return;


	if (strcmp(m->label(), "Open patch or project...") == 0) {
		__cb_open_loadpatch_win();
		return;
	}
	if (strcmp(m->label(), "Save patch...") == 0) {
		if (G_Mixer.hasLogicalSamples() || G_Mixer.hasEditedSamples())
			if (!gdConfirmWin("Warning", "You should save a project in order to store\nyour takes and/or processed samples."))
				return;
		__cb_open_savepatch_win();
		return;
	}
	if (strcmp(m->label(), "Save project...") == 0) {
		__cb_open_saveproject_win();
		return;
	}
	if (strcmp(m->label(), "Quit Giada") == 0) {
		__cb_endprogram();
		return;
	}
}


/* ------------------------------------------------------------------ */


void gdMainWindow::__cb_open_edit_menu() {
	Fl_Menu_Item menu[] = {
		{"Clear all samples"},
		{"Clear all actions"},
		{"Reset to init state"},
		{0}
	};

	/* clear all actions disabled if no recs, clear all samples disabled
	 * if no samples. */

	menu[1].deactivate();

	for (unsigned i=0; i<G_Mixer.channels.size; i++)
		if (recorder::chanEvents[i]) {
			menu[1].activate();
			break;
		}
	for (unsigned i=0; i<G_Mixer.channels.size; i++)
		if (G_Mixer.channels.at(i)->wave != NULL) {
			menu[0].activate();
			break;
		}

	Fl_Menu_Button *b = new Fl_Menu_Button(0, 0, 100, 50);
	b->box(G_BOX);
	b->textsize(11);
	b->textcolor(COLOR_TEXT_0);
	b->color(COLOR_BG_0);

	const Fl_Menu_Item *m = menu->popup(Fl::event_x(),	Fl::event_y(), 0, 0, b);
	if (!m) return;

	if (strcmp(m->label(), "Clear all samples") == 0) {
		if (!gdConfirmWin("Warning", "Clear all samples: are you sure?"))
			return;
		delSubWindow(WID_SAMPLE_EDITOR);
		glue_clearAllSamples();
		return;
	}
	if (strcmp(m->label(), "Clear all actions") == 0) {
		if (!gdConfirmWin("Warning", "Clear all actions: are you sure?"))
			return;
		delSubWindow(WID_ACTION_EDITOR);
		glue_clearAllRecs();
		return;
	}
	if (strcmp(m->label(), "Reset to init state") == 0) {
		if (!gdConfirmWin("Warning", "Reset to init state: are you sure?"))
			return;
		gu_closeAllSubwindows();
		glue_resetToInitState();
		return;
	}
}


/* ------------------------------------------------------------------ */


void gdMainWindow::__cb_metronome() {
	glue_startStopMetronome();
}


/* ------------------------------------------------------------------ */


void gdMainWindow::__cb_beatsMultiply() {
	glue_beatsMultiply();
}


/* ------------------------------------------------------------------ */


void gdMainWindow::__cb_beatsDivide() {
	glue_beatsDivide();
}


/* ------------------------------------------------------------------ */


#ifdef WITH_VST
void gdMainWindow::__cb_openMasterFxOut() {
	gu_openSubWindow(mainWin, new gdPluginList(PluginHost::MASTER_OUT), WID_FX_LIST);
}

void gdMainWindow::__cb_openMasterFxIn() {
	gu_openSubWindow(mainWin, new gdPluginList(PluginHost::MASTER_IN), WID_FX_LIST);
}

void gdMainWindow::__cb_inToOut() {
	G_Mixer.inToOut = inToOut->value();
}
#endif

