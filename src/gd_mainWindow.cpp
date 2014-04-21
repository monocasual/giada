/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
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


#include "gd_mainWindow.h"
#include "graphics.h"
#include "gd_warnings.h"
#include "glue.h"
#include "mixer.h"
#include "recorder.h"
#include "gd_browser.h"
#include "mixerHandler.h"
#include "pluginHost.h"
#include "channel.h"
#include "sampleChannel.h"
#include "gd_midiGrabber.h"


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

	size_range(GUI_WIDTH, GUI_HEIGHT);

	begin();

	menu        = new gMenu(8, -1);

				quantize    = new gChoice(632, 49, 40, 15, "", false);
				bpm         = new gClick(676,  49, 40, 15);
				beats       = new gClick(724,  49, 40, 15, "4/1");
				beats_mul   = new gClick(768,  49, 15, 15, "×");
				beats_div   = new gClick(787,  49, 15, 15, "÷");

	inOut       = new gInOut(408, 8);
	controller  = new gController(8, 39);
	beatMeter   = new gBeatMeter(100, 83, 609, 20);
	keyboard    = new Keyboard(8, 122, w()-16, 380);

	end();

	char buf_bpm[6]; snprintf(buf_bpm, 6, "%f", G_Mixer.bpm);
	bpm->copy_label(buf_bpm);
	bpm->callback(cb_change_bpm);
	beats->callback(cb_change_batt);
	beats_mul->callback(cb_beatsMultiply);
	beats_div->callback(cb_beatsDivide);

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
void gdMainWindow::cb_quantize       (Fl_Widget *v, void *p)  	{ mainWin->__cb_quantize((intptr_t)p); }
void gdMainWindow::cb_beatsMultiply  (Fl_Widget *v, void *p)    { mainWin->__cb_beatsMultiply(); }
void gdMainWindow::cb_beatsDivide    (Fl_Widget *v, void *p)    { mainWin->__cb_beatsDivide(); }


/* ------------------------------------------------------------------ */


void gdMainWindow::__cb_endprogram() {
	if (!gdConfirmWin("Warning", "Quit Giada: are you sure?"))
		return;
	init_shutdown();
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


void gdMainWindow::__cb_quantize(int v) {
	glue_quantize(v);
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
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gInOut::gInOut(int x, int y)
	: Fl_Group(x, y, 200, 20)
{
	resizable(NULL);
	begin();

#if defined(WITH_VST)
	masterFxIn  = new gButton    (x, y, 20, 20, "", fxOff_xpm, fxOn_xpm);
	inVol		    = new gDial      (masterFxIn->x()+masterFxIn->w()+4, y, 20, 20);
	inMeter     = new gSoundMeter(inVol->x()+inVol->w()+4, y+5, 140, 10);
	inToOut     = new gClick     (inMeter->x()+inMeter->w()+4, y+5, 10, 10, "");
	outMeter    = new gSoundMeter(inToOut->x()+inToOut->w()+4, y+5, 140, 10);
	outVol		  = new gDial      (outMeter->x()+outMeter->w()+4, y, 20, 20);
	masterFxOut = new gButton    (outVol->x()+outVol->w()+4, y, 20, 20, "", fxOff_xpm, fxOn_xpm);
	size(394, 20);
#else
	outMeter    = new gSoundMeter(x, y+5, 140, 10);
	inMeter     = new gSoundMeter(outMeter->x()+outMeter->w()+4, y+5, 140, 10);
	outVol		  = new gDial      (inMeter->x()+inMeter->w()+4, y, 20, 20);
	inVol		    = new gDial      (outVol->x()+outVol->w()+4, y, 20, 20);
	size(356, 20);
#endif

	end();

	outVol->callback(cb_outVol, (void*)this);
	outVol->value(G_Mixer.outVol);
	inVol->callback(cb_inVol, (void*)this);
	inVol->value(G_Mixer.inVol);

#ifdef WITH_VST
	masterFxOut->callback(cb_openMasterFxOut, (void*)this);
	masterFxIn->callback(cb_openMasterFxIn, (void*)this);
	inToOut->callback(cb_inToOut, (void*)this);
	inToOut->type(FL_TOGGLE_BUTTON);
#endif
}


/* ------------------------------------------------------------------ */


void gInOut::cb_outVol         (Fl_Widget *v, void *p)  	{ ((gInOut*)p)->__cb_outVol(); }
void gInOut::cb_inVol          (Fl_Widget *v, void *p)  	{ ((gInOut*)p)->__cb_inVol(); }
#ifdef WITH_VST
void gInOut::cb_openMasterFxOut(Fl_Widget *v, void *p)    { ((gInOut*)p)->__cb_openMasterFxOut(); }
void gInOut::cb_openMasterFxIn (Fl_Widget *v, void *p)    { ((gInOut*)p)->__cb_openMasterFxIn(); }
void gInOut::cb_inToOut        (Fl_Widget *v, void *p)    { ((gInOut*)p)->__cb_inToOut(); }
#endif


/* ------------------------------------------------------------------ */


void gInOut::__cb_outVol() {
	glue_setOutVol(outVol->value());
}


/* ------------------------------------------------------------------ */


void gInOut::__cb_inVol() {
	glue_setInVol(inVol->value());
}


/* ------------------------------------------------------------------ */


#ifdef WITH_VST
void gInOut::__cb_openMasterFxOut() {
	gu_openSubWindow(mainWin, new gdPluginList(PluginHost::MASTER_OUT), WID_FX_LIST);
}

void gInOut::__cb_openMasterFxIn() {
	gu_openSubWindow(mainWin, new gdPluginList(PluginHost::MASTER_IN), WID_FX_LIST);
}

void gInOut::__cb_inToOut() {
	G_Mixer.inToOut = inToOut->value();
}
#endif


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gMenu::gMenu(int x, int y)
	: Fl_Group(x, y, 300, 20)
{
	begin();

	file   = new gClick(x, y, 70, 21, "file");
	edit   = new gClick(file->x()+file->w()+4,  y, 70, 21, "edit");
	config = new gClick(edit->x()+edit->w()+4, y, 70, 21, "config");
	about	 = new gClick(config->x()+config->w()+4, y, 70, 21, "about");
	size(about->x()+about->w()-x, 20);

	end();

	about->callback(cb_openAboutWin, (void*)this);
	file->callback(cb_openFileMenu, (void*)this);
	edit->callback(cb_openEditMenu, (void*)this);
	config->callback(cb_openConfigWin, (void*)this);
}


/* ------------------------------------------------------------------ */


void gMenu::cb_openAboutWin (Fl_Widget *v, void *p) { ((gMenu*)p)->__cb_openAboutWin(); }
void gMenu::cb_openConfigWin(Fl_Widget *v, void *p) { ((gMenu*)p)->__cb_openConfigWin(); }
void gMenu::cb_openFileMenu (Fl_Widget *v, void *p) { ((gMenu*)p)->__cb_openFileMenu(); }
void gMenu::cb_openEditMenu (Fl_Widget *v, void *p) { ((gMenu*)p)->__cb_openEditMenu(); }


/* ------------------------------------------------------------------ */


void gMenu::__cb_openAboutWin() {
	gu_openSubWindow(mainWin, new gdAbout(), WID_ABOUT);
}


/* ------------------------------------------------------------------ */


void gMenu::__cb_openConfigWin() {
	gu_openSubWindow(mainWin, new gdConfig(380, 370), WID_CONFIG);
}


/* ------------------------------------------------------------------ */


void gMenu::__cb_openFileMenu() {

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
		gWindow *childWin = new gdBrowser("Load Patch", G_Conf.patchPath, 0, BROWSER_LOAD_PATCH);
		gu_openSubWindow(mainWin, childWin, WID_FILE_BROWSER);
		return;
	}
	if (strcmp(m->label(), "Save patch...") == 0) {
		if (G_Mixer.hasLogicalSamples() || G_Mixer.hasEditedSamples())
			if (!gdConfirmWin("Warning", "You should save a project in order to store\nyour takes and/or processed samples."))
				return;
		gWindow *childWin = new gdBrowser("Save Patch", G_Conf.patchPath, 0, BROWSER_SAVE_PATCH);
		gu_openSubWindow(mainWin, childWin, WID_FILE_BROWSER);
		return;
	}
	if (strcmp(m->label(), "Save project...") == 0) {
		gWindow *childWin = new gdBrowser("Save Project", G_Conf.patchPath, 0, BROWSER_SAVE_PROJECT);
		gu_openSubWindow(mainWin, childWin, WID_FILE_BROWSER);
		return;
	}
	if (strcmp(m->label(), "Quit Giada") == 0) {
		///__cb_endprogram();
		mainWin->do_callback();
		return;
	}
}


/* ------------------------------------------------------------------ */


void gMenu::__cb_openEditMenu() {

	Fl_Menu_Item menu[] = {
		{"Clear all samples"},
		{"Clear all actions"},
		{"Reset to init state"},
		{"Setup global MIDI input..."},
		{0}
	};

	/* clear all actions disabled if no recs, clear all samples disabled
	 * if no samples. */

	menu[1].deactivate();

	for (unsigned i=0; i<G_Mixer.channels.size; i++)
		if (G_Mixer.channels.at(i)->hasActions) {
			menu[1].activate();
			break;
		}
	for (unsigned i=0; i<G_Mixer.channels.size; i++)
		if (G_Mixer.channels.at(i)->type == CHANNEL_SAMPLE)
			if (((SampleChannel*)G_Mixer.channels.at(i))->wave != NULL) {
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
		mainWin->delSubWindow(WID_SAMPLE_EDITOR);
		glue_clearAllSamples();
		return;
	}
	if (strcmp(m->label(), "Clear all actions") == 0) {
		if (!gdConfirmWin("Warning", "Clear all actions: are you sure?"))
			return;
		mainWin->delSubWindow(WID_ACTION_EDITOR);
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
	if (strcmp(m->label(), "Setup global MIDI input...") == 0) {
		gu_openSubWindow(mainWin, new gdMidiGrabberMaster(), 0);
		return;
	}
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gController::gController(int x, int y)
	: Fl_Group(x, y, 300, 20)
{
	begin();

	rewind    = new gClick(x,  y, 25, 25, "", rewindOff_xpm, rewindOn_xpm);
	play      = new gClick(rewind->x()+rewind->w()+4, y, 25, 25, "", play_xpm, pause_xpm);
	recAction = new gClick(play->x()+play->w()+4, y, 25, 25, "", recOff_xpm, recOn_xpm);
	recInput  = new gClick(recAction->x()+recAction->w()+4, y, 25, 25, "", inputRecOff_xpm, inputRecOn_xpm);
	metronome = new gClick(recInput->x()+recInput->w()+4, y+10, 15, 15, "", metronomeOff_xpm, metronomeOn_xpm);

	end();

	rewind->callback(cb_rewind, (void*)this);

	play->callback(cb_play);
	play->type(FL_TOGGLE_BUTTON);

	recAction->callback(cb_recAction, (void*)this);
	recAction->type(FL_TOGGLE_BUTTON);

	recInput->callback(cb_recInput, (void*)this);
	recInput->type(FL_TOGGLE_BUTTON);

	metronome->callback(cb_metronome);
	metronome->type(FL_TOGGLE_BUTTON);
}


/* ------------------------------------------------------------------ */


void gController::cb_rewind   (Fl_Widget *v, void *p) { ((gController*)p)->__cb_rewind(); }
void gController::cb_play     (Fl_Widget *v, void *p) { ((gController*)p)->__cb_play(); }
void gController::cb_recAction(Fl_Widget *v, void *p) { ((gController*)p)->__cb_recAction(); }
void gController::cb_recInput (Fl_Widget *v, void *p) { ((gController*)p)->__cb_recInput(); }
void gController::cb_metronome(Fl_Widget *v, void *p) { ((gController*)p)->__cb_metronome(); }


/* ------------------------------------------------------------------ */


void gController::__cb_rewind() {
	glue_rewindSeq();
}


/* ------------------------------------------------------------------ */


void gController::__cb_play() {
	glue_startStopSeq();
}


/* ------------------------------------------------------------------ */


void gController::__cb_recAction() {
	glue_startStopActionRec();
}


/* ------------------------------------------------------------------ */


void gController::__cb_recInput() {
	glue_startStopInputRec();
}


/* ------------------------------------------------------------------ */


void gController::__cb_metronome() {
	glue_startStopMetronome();
}


/* ------------------------------------------------------------------ */


void gController::updatePlay(int v) {
	play->value(v);
	play->redraw();
}


/* ------------------------------------------------------------------ */


void gController::updateMetronome(int v) {
	metronome->value(v);
	metronome->redraw();
}


/* ------------------------------------------------------------------ */


void gController::updateRecInput(int v) {
	recInput->value(v);
	recInput->redraw();
}


/* ------------------------------------------------------------------ */


void gController::updateRecAction(int v) {
	recAction->value(v);
	recAction->redraw();
}
