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


#ifdef __linux__
	#include <sys/stat.h>			// for mkdir
#endif

#include "gd_mainWindow.h"
#include "gd_warnings.h"
#include "gd_bpmInput.h"
#include "gd_beatsInput.h"
#include "gd_midiGrabber.h"
#include "gg_keyboard.h"
#include "gd_about.h"
#include "gd_config.h"
#include "gd_browser.h"
#include "graphics.h"
#include "glue.h"
#include "mixer.h"
#include "recorder.h"
#include "mixerHandler.h"
#include "pluginHost.h"
#include "channel.h"
#include "sampleChannel.h"
#include "init.h"
#include "patch.h"
#include "conf.h"

#ifdef WITH_VST
#include "gd_pluginList.h"
#endif


extern Mixer	   		 G_Mixer;
extern Patch     		 G_Patch;
extern Conf	 	   		 G_Conf;
extern gdMainWindow *mainWin;
extern bool	 		 		 G_quit;
extern bool 		 		 G_audio_status;

#if defined(WITH_VST)
extern PluginHost  	 G_PluginHost;
#endif


gdMainWindow::gdMainWindow(int W, int H, const char *title, int argc, char **argv)
	: gWindow(W, H, title)
{
	Fl::visible_focus(0);
	Fl::background(25, 25, 25);
	Fl::set_boxtype(G_BOX, gDrawBox, 1, 1, 2, 2);    // custom box G_BOX

	size_range(GUI_WIDTH, GUI_HEIGHT);

	menu       = new gMenu(8, -1);
	inOut      = new gInOut(408, 8);	
	controller = new gController(8, 39);
	timing     = new gTiming(632, 39);
	beatMeter  = new gBeatMeter(100, 83, 609, 20);
	keyboard   = new gKeyboard(8, 122, w()-16, 380);
	
	/* zone 1 - menus, and I/O tools */
	
	Fl_Group *zone1 = new Fl_Group(8, 8, W-16, 20);	
	zone1->add(menu);
	zone1->resizable(new Fl_Box(300, 8, 80, 20));
	zone1->add(inOut);

	/* zone 2 - controller and timing tools */
	
	Fl_Group *zone2 = new Fl_Group(8, controller->y(), W-16, controller->h());	
	zone2->add(controller);
	zone2->resizable(new Fl_Box(controller->x()+controller->w()+4, zone2->y(), 80, 20));
	zone2->add(timing);

	/* zone 3 - beat meter */
	
	Fl_Group *zone3 = new Fl_Group(8, beatMeter->y(), W-16, beatMeter->h());	
	zone3->add(beatMeter);
	
	/* zone 4 - the keyboard (Fl_Group is unnecessary here, keyboard is
	 * a group by itself) */

	resizable(keyboard);
	
	add(zone1);
	add(zone2);
	add(zone3);
	add(keyboard);
	callback(cb_endprogram);
	gu_setFavicon(this);
	show(argc, argv);
}

/*
gdMainWindow::~gdMainWindow() {}
*/

/* ------------------------------------------------------------------ */


void gdMainWindow::cb_endprogram(Fl_Widget *v, void *p) { mainWin->__cb_endprogram(); }


/* ------------------------------------------------------------------ */


void gdMainWindow::__cb_endprogram()
{
	if (!gdConfirmWin("Warning", "Quit Giada: are you sure?"))
		return;
	init_shutdown();
	hide();
	delete this;
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gInOut::gInOut(int x, int y)
	: Fl_Group(x, y, 400, 20)
{
	begin();
	
#if defined(WITH_VST)
	masterFxIn  = new gButton    (x, y, 20, 20, "", fxOff_xpm, fxOn_xpm);
	inVol		    = new gDial      (masterFxIn->x()+masterFxIn->w()+4, y, 20, 20);
	inMeter     = new gSoundMeter(inVol->x()+inVol->w()+4, y+5, 140, 10);
	inToOut     = new gClick     (inMeter->x()+inMeter->w()+4, y+5, 10, 10, "");
	outMeter    = new gSoundMeter(inToOut->x()+inToOut->w()+4, y+5, 140, 10);
	outVol		  = new gDial      (outMeter->x()+outMeter->w()+4, y, 20, 20);
	masterFxOut = new gButton    (outVol->x()+outVol->w()+4, y, 20, 20, "", fxOff_xpm, fxOn_xpm);
#else
	outMeter    = new gSoundMeter(x, y+5, 140, 10);
	inMeter     = new gSoundMeter(outMeter->x()+outMeter->w()+4, y+5, 140, 10);
	outVol		  = new gDial      (inMeter->x()+inMeter->w()+4, y, 20, 20);
	inVol		    = new gDial      (outVol->x()+outVol->w()+4, y, 20, 20);
#endif

	end();
	
	resizable(NULL);   // don't resize any widget

	outVol->callback(cb_outVol, (void*)this);
	outVol->value(G_Mixer.outVol);
	inVol->callback(cb_inVol, (void*)this);
	inVol->value(G_Mixer.inVol);

#ifdef WITH_VST
	masterFxOut->callback(cb_masterFxOut, (void*)this);
	masterFxIn->callback(cb_masterFxIn, (void*)this);
	inToOut->callback(cb_inToOut, (void*)this);
	inToOut->type(FL_TOGGLE_BUTTON);
#endif
}


/* ------------------------------------------------------------------ */


void gInOut::cb_outVol     (Fl_Widget *v, void *p)  	{ ((gInOut*)p)->__cb_outVol(); }
void gInOut::cb_inVol      (Fl_Widget *v, void *p)  	{ ((gInOut*)p)->__cb_inVol(); }
#ifdef WITH_VST
void gInOut::cb_masterFxOut(Fl_Widget *v, void *p)    { ((gInOut*)p)->__cb_masterFxOut(); }
void gInOut::cb_masterFxIn (Fl_Widget *v, void *p)    { ((gInOut*)p)->__cb_masterFxIn(); }
void gInOut::cb_inToOut    (Fl_Widget *v, void *p)    { ((gInOut*)p)->__cb_inToOut(); }
#endif


/* ------------------------------------------------------------------ */


void gInOut::__cb_outVol() 
{
	glue_setOutVol(outVol->value());
}


/* ------------------------------------------------------------------ */


void gInOut::__cb_inVol() 
{
	glue_setInVol(inVol->value());
}


/* ------------------------------------------------------------------ */


#ifdef WITH_VST
void gInOut::__cb_masterFxOut() 
{
	gu_openSubWindow(mainWin, new gdPluginList(PluginHost::MASTER_OUT), WID_FX_LIST);
}

void gInOut::__cb_masterFxIn() 
{
	gu_openSubWindow(mainWin, new gdPluginList(PluginHost::MASTER_IN), WID_FX_LIST);
}

void gInOut::__cb_inToOut() 
{
	G_Mixer.inToOut = inToOut->value();
}
#endif


/* ------------------------------------------------------------------ */


void gInOut::refresh()
{
	outMeter->mixerPeak = G_Mixer.peakOut;
	inMeter->mixerPeak  = G_Mixer.peakIn;
	outMeter->redraw();
	inMeter->redraw();		
}


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

	end();

	resizable(NULL);   // don't resize any widget

	about->callback(cb_about, (void*)this);
	file->callback(cb_file, (void*)this);
	edit->callback(cb_edit, (void*)this);
	config->callback(cb_config, (void*)this);
}


/* ------------------------------------------------------------------ */


void gMenu::cb_about (Fl_Widget *v, void *p) { ((gMenu*)p)->__cb_about(); }
void gMenu::cb_config(Fl_Widget *v, void *p) { ((gMenu*)p)->__cb_config(); }
void gMenu::cb_file  (Fl_Widget *v, void *p) { ((gMenu*)p)->__cb_file(); }
void gMenu::cb_edit  (Fl_Widget *v, void *p) { ((gMenu*)p)->__cb_edit(); }


/* ------------------------------------------------------------------ */


void gMenu::__cb_about() 
{
	gu_openSubWindow(mainWin, new gdAbout(), WID_ABOUT);
}


/* ------------------------------------------------------------------ */


void gMenu::__cb_config() 
{
	gu_openSubWindow(mainWin, new gdConfig(380, 370), WID_CONFIG);
}


/* ------------------------------------------------------------------ */


void gMenu::__cb_file() 
{
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
		mainWin->do_callback();
		return;
	}
}


/* ------------------------------------------------------------------ */


void gMenu::__cb_edit() 
{
	Fl_Menu_Item menu[] = {
		{"Clear all samples"},
		{"Clear all actions"},
		{"Remove empty columns"},
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
	if (strcmp(m->label(), "Remove empty columns") == 0) {
		mainWin->keyboard->organizeColumns();
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
	: Fl_Group(x, y, 131, 25)
{
	begin();

	rewind    = new gClick(x,  y, 25, 25, "", rewindOff_xpm, rewindOn_xpm);
	play      = new gClick(rewind->x()+rewind->w()+4, y, 25, 25, "", play_xpm, pause_xpm);
	recAction = new gClick(play->x()+play->w()+4, y, 25, 25, "", recOff_xpm, recOn_xpm);
	recInput  = new gClick(recAction->x()+recAction->w()+4, y, 25, 25, "", inputRecOff_xpm, inputRecOn_xpm);
	metronome = new gClick(recInput->x()+recInput->w()+4, y+10, 15, 15, "", metronomeOff_xpm, metronomeOn_xpm);

	end();

	resizable(NULL);   // don't resize any widget

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


void gController::__cb_rewind() 
{
	glue_rewindSeq();
}


/* ------------------------------------------------------------------ */


void gController::__cb_play() 
{
	glue_startStopSeq();
}


/* ------------------------------------------------------------------ */


void gController::__cb_recAction() 
{
	glue_startStopActionRec();
}


/* ------------------------------------------------------------------ */


void gController::__cb_recInput() 
{
	glue_startStopInputRec();
}


/* ------------------------------------------------------------------ */


void gController::__cb_metronome() 
{
	glue_startStopMetronome();
}


/* ------------------------------------------------------------------ */


void gController::updatePlay(int v) 
{
	play->value(v);
	play->redraw();
}


/* ------------------------------------------------------------------ */


void gController::updateMetronome(int v) 
{
	metronome->value(v);
	metronome->redraw();
}


/* ------------------------------------------------------------------ */


void gController::updateRecInput(int v) 
{
	recInput->value(v);
	recInput->redraw();
}


/* ------------------------------------------------------------------ */


void gController::updateRecAction(int v) 
{
	recAction->value(v);
	recAction->redraw();
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gTiming::gTiming(int x, int y)
	: Fl_Group(x, y, 170, 15)
{
	begin();

	quantizer  = new gChoice(x, y, 40, 15, "", false);
	bpm        = new gClick (quantizer->x()+quantizer->w()+4,  y, 40, 15);
	meter      = new gClick (bpm->x()+bpm->w()+8,  y, 40, 15, "4/1");
	multiplier = new gClick (meter->x()+meter->w()+4, y, 15, 15, "×");
	divider    = new gClick (multiplier->x()+multiplier->w()+4, y, 15, 15, "÷");

	end();
	
	resizable(NULL);   // don't resize any widget

	char buf[6]; snprintf(buf, 6, "%f", G_Mixer.bpm);
	bpm->copy_label(buf);
	
	bpm->callback(cb_bpm, (void*)this);
	meter->callback(cb_meter, (void*)this);
	multiplier->callback(cb_multiplier, (void*)this);
	divider->callback(cb_divider, (void*)this);

	quantizer->add("off", 0, cb_quantizer, (void*)this);
	quantizer->add("1b",  0, cb_quantizer, (void*)this);
	quantizer->add("2b",  0, cb_quantizer, (void*)this);
	quantizer->add("3b",  0, cb_quantizer, (void*)this);
	quantizer->add("4b",  0, cb_quantizer, (void*)this);
	quantizer->add("6b",  0, cb_quantizer, (void*)this);
	quantizer->add("8b",  0, cb_quantizer, (void*)this);
	quantizer->value(0); //  "off" by default
}


/* ------------------------------------------------------------------ */


void gTiming::cb_bpm       (Fl_Widget *v, void *p) { ((gTiming*)p)->__cb_bpm(); }
void gTiming::cb_meter     (Fl_Widget *v, void *p) { ((gTiming*)p)->__cb_meter(); }
void gTiming::cb_quantizer (Fl_Widget *v, void *p) { ((gTiming*)p)->__cb_quantizer(); }
void gTiming::cb_multiplier(Fl_Widget *v, void *p) { ((gTiming*)p)->__cb_multiplier(); }
void gTiming::cb_divider   (Fl_Widget *v, void *p) { ((gTiming*)p)->__cb_divider(); }


/* ------------------------------------------------------------------ */


void gTiming::__cb_bpm() 
{
	gu_openSubWindow(mainWin, new gdBpmInput(bpm->label()), WID_BPM);
}


/* ------------------------------------------------------------------ */


void gTiming::__cb_meter() 
{
	gu_openSubWindow(mainWin, new gdBeatsInput(), WID_BEATS);
}


/* ------------------------------------------------------------------ */


void gTiming::__cb_quantizer() 
{
	glue_quantize(quantizer->value());
}


/* ------------------------------------------------------------------ */


void gTiming::__cb_multiplier() 
{
	glue_beatsMultiply();
}


/* ------------------------------------------------------------------ */


void gTiming::__cb_divider() 
{
	glue_beatsDivide();
}


/* ------------------------------------------------------------------ */


void gTiming::setBpm(const char *v) 
{ 
	bpm->copy_label(v); 
}


/* ------------------------------------------------------------------ */


void gTiming::setMeter(int beats, int bars)
{
	char buf[8];
	sprintf(buf, "%d/%d", beats, bars);
	meter->copy_label(buf);
}
