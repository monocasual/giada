/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gg_keyboard
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


#include "gg_keyboard.h"
#include "gd_browser.h"
#include "const.h"
#include "mixer.h"
#include "wave.h"
#include "gd_editor.h"
#include "conf.h"
#include "patch.h"
#include "gd_mainWindow.h"
#include "graphics.h"
#include "glue.h"
#include "recorder.h"
#include "gd_warnings.h"
#include "pluginHost.h"
#include "channel.h"
#include "sampleChannel.h"
#include "midiChannel.h"
#include "gd_keyGrabber.h"
#include "gd_midiGrabber.h"
#include "gd_midiOutputSetup.h"
#include "log.h"


extern Mixer 		     G_Mixer;
extern Conf  		     G_Conf;
extern Patch 		     G_Patch;
extern gdMainWindow *mainWin;


gChannel::gChannel(int X, int Y, int W, int H)
 : Fl_Group(X, Y, W, H, NULL)
{
}


/* ------------------------------------------------------------------ */


gSampleChannel::gSampleChannel(int X, int Y, int W, int H, class SampleChannel *ch)
	: gChannel(X, Y, W, H), ch(ch)
{
	begin();

	button       = new gButton (x(), y(), 20, 20);
	status       = new gStatus (button->x()+button->w()+4, y(), 20, 20, ch);

#if defined(WITH_VST)
	sampleButton = new gClick  (status->x()+status->w()+4, y(), 213, 20, "-- no sample --");
#else
	sampleButton = new gClick  (status->x()+status->w()+4, y(), 237, 20, "-- no sample --");
#endif
	modeBox      = new gModeBox(sampleButton->x()+sampleButton->w()+4, y(), 20, 20, ch);
	mute         = new gClick  (modeBox->x()+modeBox->w()+4, y(), 20, 20, "", muteOff_xpm, muteOn_xpm);
	solo         = new gClick  (mute->x()+mute->w()+4, y(), 20, 20, "", soloOff_xpm, soloOn_xpm);

#if defined(WITH_VST)
	fx           = new gButton (solo->x()+solo->w()+4, y(), 20, 20, "", fxOff_xpm, fxOn_xpm);
	vol          = new gDial   (fx->x()+fx->w()+4, y(), 20, 20);
#else
	vol          = new gDial   (solo->x()+solo->w()+4, y(), 20, 20);
#endif

	readActions  = NULL; // no 'R' button

	end();

	update();

	button->callback(cb_button, (void*)this);
	button->when(FL_WHEN_CHANGED);   // do callback on keypress && on keyrelease

#ifdef WITH_VST
	fx->callback(cb_openFxWindow, (void*)this);
#endif

	mute->type(FL_TOGGLE_BUTTON);
	mute->callback(cb_mute, (void*)this);

	solo->type(FL_TOGGLE_BUTTON);
	solo->callback(cb_solo, (void*)this);

	sampleButton->callback(cb_openMenu, (void*)this);
	vol->callback(cb_changeVol, (void*)this);

	ch->guiChannel = this;
}


/* ------------------------------------------------------------------ */


void gSampleChannel::cb_button      (Fl_Widget *v, void *p) { ((gSampleChannel*)p)->__cb_button(); }
void gSampleChannel::cb_mute        (Fl_Widget *v, void *p) { ((gSampleChannel*)p)->__cb_mute(); }
void gSampleChannel::cb_solo        (Fl_Widget *v, void *p) { ((gSampleChannel*)p)->__cb_solo(); }
void gSampleChannel::cb_openMenu    (Fl_Widget *v, void *p) { ((gSampleChannel*)p)->__cb_openMenu(); }
void gSampleChannel::cb_changeVol   (Fl_Widget *v, void *p) { ((gSampleChannel*)p)->__cb_changeVol(); }
void gSampleChannel::cb_readActions (Fl_Widget *v, void *p) { ((gSampleChannel*)p)->__cb_readActions(); }
#ifdef WITH_VST
void gSampleChannel::cb_openFxWindow(Fl_Widget *v, void *p) { ((gSampleChannel*)p)->__cb_openFxWindow(); }
#endif


/* ------------------------------------------------------------------ */


void gSampleChannel::__cb_mute() {
	glue_setMute(ch);
}


/* ------------------------------------------------------------------ */


void gSampleChannel::__cb_solo() {
	solo->value() ? glue_setSoloOn(ch) : glue_setSoloOff(ch);
}


/* ------------------------------------------------------------------ */


void gSampleChannel::__cb_changeVol() {
	glue_setChanVol(ch, vol->value());
}


/* ------------------------------------------------------------------ */


#ifdef WITH_VST
void gSampleChannel::__cb_openFxWindow() {
	gu_openSubWindow(mainWin, new gdPluginList(PluginHost::CHANNEL, ch), WID_FX_LIST);
}
#endif

/* ------------------------------------------------------------------ */



void gSampleChannel::__cb_button() {
	if (button->value())    // pushed
		glue_keyPress(ch, Fl::event_ctrl(), Fl::event_shift());
	else                    // released
		glue_keyRelease(ch, Fl::event_ctrl(), Fl::event_shift());
}


/* ------------------------------------------------------------------ */


void gSampleChannel::__cb_openMenu() {

	/* if you're recording (actions or input) no menu is allowed; you can't
	 * do anything, especially deallocate the channel */

	if (G_Mixer.chanInput == ch || recorder::active)
		return;

	/* the following is a trash workaround for a FLTK menu. We need a gMenu
	 * widget asap */

	Fl_Menu_Item rclick_menu[] = {
		{"Load new sample..."},                     // 0
		{"Export sample to file..."},               // 1
		{"Setup keyboard input..."},                // 2
		{"Setup MIDI input..."},                    // 3
		{"Edit sample..."},                         // 4
		{"Edit actions..."},                        // 5
		{"Clear actions", 0, 0, 0, FL_SUBMENU},     // 6
			{"All"},                                  // 7
			{"Mute"},                                 // 8
			{"Volume"},                               // 9
			{"Start/Stop"},                           // 10
			{0},                                      // 11
		{"Free channel"},                           // 12
		{"Delete channel"},                         // 13
		{0}
	};

	if (ch->status & (STATUS_EMPTY | STATUS_MISSING)) {
		rclick_menu[1].deactivate();
		rclick_menu[4].deactivate();
		rclick_menu[12].deactivate();
	}

	/* no 'clear actions' if there are no actions */

	if (!ch->hasActions)
		rclick_menu[6].deactivate();

	/* no 'clear start/stop actions' for those channels in loop mode:
	 * they cannot have start/stop actions. */

	if (ch->mode & LOOP_ANY)
		rclick_menu[10].deactivate();

	Fl_Menu_Button *b = new Fl_Menu_Button(0, 0, 100, 50);
	b->box(G_BOX);
	b->textsize(11);
	b->textcolor(COLOR_TEXT_0);
	b->color(COLOR_BG_0);

	const Fl_Menu_Item *m = rclick_menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, b);
	if (!m) return;

	if (strcmp(m->label(), "Load new sample...") == 0) {
		openBrowser(BROWSER_LOAD_SAMPLE);
		return;
	}

	if (strcmp(m->label(), "Setup keyboard input...") == 0) {
		new gdKeyGrabber(ch); /// FIXME - use gu_openSubWindow
		return;
	}

	if (strcmp(m->label(), "Setup MIDI input...") == 0) {
		gu_openSubWindow(mainWin, new gdMidiGrabberChannel(ch), 0);
		return;
	}

	if (strcmp(m->label(), "Edit sample...") == 0) {
		gu_openSubWindow(mainWin, new gdEditor(ch), WID_SAMPLE_EDITOR); /// FIXME title it's up to gdEditor
		return;
	}

	if (strcmp(m->label(), "Export sample to file...") == 0) {
		openBrowser(BROWSER_SAVE_SAMPLE);
		return;
	}

	if (strcmp(m->label(), "Delete channel") == 0) {
		if (!gdConfirmWin("Warning", "Delete channel: are you sure?"))
			return;
		glue_deleteChannel(ch);
		return;
	}

	if (strcmp(m->label(), "Free channel") == 0) {
		if (ch->status == STATUS_PLAY) {
			if (!gdConfirmWin("Warning", "This action will stop the channel: are you sure?"))
				return;
		}
		else if (!gdConfirmWin("Warning", "Free channel: are you sure?"))
			return;

		glue_freeChannel(ch);

		/* delete any related subwindow */

		/** FIXME - use gu_closeAllSubwindows() */

		mainWin->delSubWindow(WID_FILE_BROWSER);
		mainWin->delSubWindow(WID_ACTION_EDITOR);
		mainWin->delSubWindow(WID_SAMPLE_EDITOR);
		mainWin->delSubWindow(WID_FX_LIST);

		return;
	}

	if (strcmp(m->label(), "Mute") == 0) {
		if (!gdConfirmWin("Warning", "Clear all mute actions: are you sure?"))
			return;
		recorder::clearAction(ch->index, ACTION_MUTEON | ACTION_MUTEOFF);
		if (!ch->hasActions)
			delActionButton();

		/* TODO - set mute=false */

		gu_refreshActionEditor(); // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "Start/Stop") == 0) {
		if (!gdConfirmWin("Warning", "Clear all start/stop actions: are you sure?"))
			return;
		recorder::clearAction(ch->index, ACTION_KEYPRESS | ACTION_KEYREL | ACTION_KILLCHAN);
		if (!ch->hasActions)
			delActionButton();
		gu_refreshActionEditor();  // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "Volume") == 0) {
		if (!gdConfirmWin("Warning", "Clear all volume actions: are you sure?"))
			return;
		recorder::clearAction(ch->index, ACTION_VOLUME);
		if (!ch->hasActions)
			delActionButton();
		gu_refreshActionEditor();  // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "All") == 0) {
		if (!gdConfirmWin("Warning", "Clear all actions: are you sure?"))
			return;
		recorder::clearChan(ch->index);
		delActionButton();
		gu_refreshActionEditor(); // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "Edit actions...") == 0) {
		gu_openSubWindow(mainWin, new gdActionEditor(ch),	WID_ACTION_EDITOR);
		return;
	}
}


/* ------------------------------------------------------------------ */


void gSampleChannel::__cb_readActions() {
	ch->readActions ? glue_stopReadingRecs(ch) : glue_startReadingRecs(ch);
}


/* ------------------------------------------------------------------ */


void gSampleChannel::openBrowser(int type) {
	const char *title = "";
	switch (type) {
		case BROWSER_LOAD_SAMPLE:
			title = "Browse Sample";
			break;
		case BROWSER_SAVE_SAMPLE:
			title = "Save Sample";
			break;
		case -1:
			title = "Edit Sample";
			break;
	}
	gWindow *childWin = new gdBrowser(title, G_Conf.samplePath, ch, type);
	gu_openSubWindow(mainWin, childWin,	WID_FILE_BROWSER);
}


/* ------------------------------------------------------------------ */


void gSampleChannel::refresh() {

	if (ch->status == STATUS_OFF) {
		sampleButton->bgColor0 = COLOR_BG_0;
		sampleButton->bdColor  = COLOR_BD_0;
		sampleButton->txtColor = COLOR_TEXT_0;
	}
	else
	if (ch->status == STATUS_PLAY) {
		sampleButton->bgColor0 = COLOR_BG_2;
		sampleButton->bdColor  = COLOR_BD_1;
		sampleButton->txtColor = COLOR_TEXT_1;
	}
	else
	if (ch->status & (STATUS_WAIT | STATUS_ENDING))
		__gu_blinkChannel(this);    /// TODO - move to gChannel::blink

	if (ch->recStatus & (REC_WAITING | REC_ENDING))
		__gu_blinkChannel(this);    /// TODO - move to gChannel::blink

	if (ch->wave != NULL) {

		if (G_Mixer.chanInput == ch)
			sampleButton->bgColor0 = COLOR_BG_3;

		if (recorder::active) {
			if (recorder::canRec(ch)) {
				sampleButton->bgColor0 = COLOR_BG_4;
				sampleButton->txtColor = COLOR_TEXT_0;
			}
		}
		status->redraw();
	}
	sampleButton->redraw();
}


/* ------------------------------------------------------------------ */


void gSampleChannel::reset() {
	sampleButton->bgColor0 = COLOR_BG_0;
	sampleButton->bdColor  = COLOR_BD_0;
	sampleButton->txtColor = COLOR_TEXT_0;
	sampleButton->label("-- no sample --");
	delActionButton(true); // force==true, don't check, just remove it
 	sampleButton->redraw();
	status->redraw();
}


/* ------------------------------------------------------------------ */


void gSampleChannel::update() {

	/* update sample button's label */

	switch (ch->status) {
		case STATUS_EMPTY:
			sampleButton->label("-- no sample --");
			break;
		case STATUS_MISSING:
		case STATUS_WRONG:
			sampleButton->label("* file not found! *");
			break;
		default:
			gu_trim_label(ch->wave->name.c_str(), 28, sampleButton);
			break;
	}

	/* update channels. If you load a patch with recorded actions, the 'R'
	 * button must be shown. Moreover if the actions are active, the 'R'
	 * button must be activated accordingly. */

	if (ch->hasActions)
		addActionButton();
	else
		delActionButton();

	/* update key box */

	char k[4];
	sprintf(k, "%c", ch->key);
	button->copy_label(k);
	button->redraw();

	/* updates modebox */

	modeBox->value(ch->mode);
	modeBox->redraw();

	/* update volumes+mute+solo */

	vol->value(ch->volume);
	mute->value(ch->mute);
	solo->value(ch->solo);
}


/* ------------------------------------------------------------------ */


int gSampleChannel::keyPress(int e) {
	int ret;
	if (e == FL_KEYDOWN && button->value())                              // key already pressed! skip it
		ret = 1;
	else
	if (Fl::event_key() == ch->key && !button->value()) {
		button->take_focus();                                              // move focus to this button
		button->value((e == FL_KEYDOWN || e == FL_SHORTCUT) ? 1 : 0);      // change the button's state
		button->do_callback();                                             // invoke the button's callback
		ret = 1;
	}
	else
		ret = 0;

	if (Fl::event_key() == ch->key)
		button->value((e == FL_KEYDOWN || e == FL_SHORTCUT) ? 1 : 0);      // change the button's state

	return ret;
}


/* ------------------------------------------------------------------ */


void gSampleChannel::addActionButton() {

	/* quit if 'R' exists yet. */

	if (readActions != NULL)
		return;

	sampleButton->size(sampleButton->w()-24, sampleButton->h());

	redraw();

	readActions = new gClick(sampleButton->x() + sampleButton->w() + 4, sampleButton->y(), 20, 20, "", readActionOff_xpm, readActionOn_xpm);
	readActions->type(FL_TOGGLE_BUTTON);
	readActions->value(ch->readActions);
	readActions->callback(cb_readActions, (void*)this);
	add(readActions);

	/* hard redraw: there's no other way to avoid glitches when moving
	 * the 'R' button */

	mainWin->keyboard->redraw();
}


/* ------------------------------------------------------------------ */


void gSampleChannel::delActionButton(bool force) {
	if (!force && (readActions == NULL || ch->hasActions))
		return;

	remove(readActions);		// delete from Keyboard group (FLTK)
	delete readActions;     // delete (C++)
	readActions = NULL;

	sampleButton->size(sampleButton->w()+24, sampleButton->h());
	sampleButton->redraw();
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gMidiChannel::gMidiChannel(int X, int Y, int W, int H, class MidiChannel *ch)
	: gChannel(X, Y, W, H), ch(ch)
{
	begin();

	button       = new gButton (x(), y(), 20, 20);

#if defined(WITH_VST)
	sampleButton = new gClick (button->x()+button->w()+4, y(), 261, 20, "-- MIDI --");
#else
	sampleButton = new gClick (button->x()+button->w()+4, y(), 285, 20, "-- MIDI --");
#endif

	mute         = new gClick (sampleButton->x()+sampleButton->w()+4, y(), 20, 20, "", muteOff_xpm, muteOn_xpm);
	solo         = new gClick (mute->x()+mute->w()+4, y(), 20, 20, "", soloOff_xpm, soloOn_xpm);

#if defined(WITH_VST)
	fx           = new gButton(solo->x()+solo->w()+4, y(), 20, 20, "", fxOff_xpm, fxOn_xpm);
	vol          = new gDial  (fx->x()+fx->w()+4, y(), 20, 20);
#else
	vol          = new gDial  (solo->x()+solo->w()+4, y(), 20, 20);
#endif

	end();

	update();

	button->callback(cb_button, (void*)this);
	button->when(FL_WHEN_CHANGED);   // do callback on keypress && on keyrelease

#ifdef WITH_VST
	fx->callback(cb_openFxWindow, (void*)this);
#endif

	mute->type(FL_TOGGLE_BUTTON);
	mute->callback(cb_mute, (void*)this);

	solo->type(FL_TOGGLE_BUTTON);
	solo->callback(cb_solo, (void*)this);

	sampleButton->callback(cb_openMenu, (void*)this);
	vol->callback(cb_changeVol, (void*)this);

	ch->guiChannel = this;
}


/* ------------------------------------------------------------------ */


void gMidiChannel::cb_button      (Fl_Widget *v, void *p) { ((gMidiChannel*)p)->__cb_button(); }
void gMidiChannel::cb_mute        (Fl_Widget *v, void *p) { ((gMidiChannel*)p)->__cb_mute(); }
void gMidiChannel::cb_solo        (Fl_Widget *v, void *p) { ((gMidiChannel*)p)->__cb_solo(); }
void gMidiChannel::cb_openMenu    (Fl_Widget *v, void *p) { ((gMidiChannel*)p)->__cb_openMenu(); }
void gMidiChannel::cb_changeVol   (Fl_Widget *v, void *p) { ((gMidiChannel*)p)->__cb_changeVol(); }
#ifdef WITH_VST
void gMidiChannel::cb_openFxWindow(Fl_Widget *v, void *p) { ((gMidiChannel*)p)->__cb_openFxWindow(); }
#endif


/* ------------------------------------------------------------------ */


void gMidiChannel::__cb_mute() {
	glue_setMute(ch);
}


/* ------------------------------------------------------------------ */


void gMidiChannel::__cb_solo() {
	solo->value() ? glue_setSoloOn(ch) : glue_setSoloOff(ch);
}


/* ------------------------------------------------------------------ */


void gMidiChannel::__cb_changeVol() {
	glue_setChanVol(ch, vol->value());
}


/* ------------------------------------------------------------------ */


#ifdef WITH_VST
void gMidiChannel::__cb_openFxWindow() {
	gu_openSubWindow(mainWin, new gdPluginList(PluginHost::CHANNEL, ch), WID_FX_LIST);
}
#endif

/* ------------------------------------------------------------------ */


void gMidiChannel::__cb_button() {
	if (button->value())
		glue_keyPress(ch, Fl::event_ctrl(), Fl::event_shift());
}


/* ------------------------------------------------------------------ */


void gMidiChannel::__cb_openMenu() {

	Fl_Menu_Item rclick_menu[] = {
		{"Edit actions..."},                        // 0
		{"Clear actions", 0, 0, 0, FL_SUBMENU},     // 1
			{"All"},                                  // 2
			{0},                                      // 3
		{"Setup MIDI output..."},                   // 4
		{"Setup MIDI input..."},                    // 5
		{"Delete channel"},                         // 6
		{0}
	};

	/* no 'clear actions' if there are no actions */

	if (!ch->hasActions)
		rclick_menu[1].deactivate();

	Fl_Menu_Button *b = new Fl_Menu_Button(0, 0, 100, 50);
	b->box(G_BOX);
	b->textsize(11);
	b->textcolor(COLOR_TEXT_0);
	b->color(COLOR_BG_0);

	const Fl_Menu_Item *m = rclick_menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, b);
	if (!m) return;

	if (strcmp(m->label(), "Delete channel") == 0) {
		if (!gdConfirmWin("Warning", "Delete channel: are you sure?"))
			return;
		glue_deleteChannel(ch);
		return;
	}

	if (strcmp(m->label(), "All") == 0) {
		if (!gdConfirmWin("Warning", "Clear all actions: are you sure?"))
			return;
		recorder::clearChan(ch->index);
		gu_refreshActionEditor(); // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "Edit actions...") == 0) {
		gu_openSubWindow(mainWin, new gdActionEditor(ch),	WID_ACTION_EDITOR);
		return;
	}

	if (strcmp(m->label(), "Setup MIDI output...") == 0) {
		gu_openSubWindow(mainWin, new gdMidiOutputSetup(ch), 0);
		return;
	}

	if (strcmp(m->label(), "Setup MIDI input...") == 0) {
		gu_openSubWindow(mainWin, new gdMidiGrabberChannel(ch), 0);
		return;
	}
}


/* ------------------------------------------------------------------ */


void gMidiChannel::refresh() {

	if (ch->status == STATUS_OFF) {
		sampleButton->bgColor0 = COLOR_BG_0;
		sampleButton->bdColor  = COLOR_BD_0;
		sampleButton->txtColor = COLOR_TEXT_0;
	}
	else
	if (ch->status == STATUS_PLAY) {
		sampleButton->bgColor0 = COLOR_BG_2;
		sampleButton->bdColor  = COLOR_BD_1;
		sampleButton->txtColor = COLOR_TEXT_1;
	}
	else
	if (ch->status & (STATUS_WAIT | STATUS_ENDING))
		__gu_blinkChannel(this);    /// TODO - move to gChannel::blink

	if (ch->recStatus & (REC_WAITING | REC_ENDING))
		__gu_blinkChannel(this);    /// TODO - move to gChannel::blink

	sampleButton->redraw();
}


/* ------------------------------------------------------------------ */


void gMidiChannel::reset() {
	sampleButton->bgColor0 = COLOR_BG_0;
	sampleButton->bdColor  = COLOR_BD_0;
	sampleButton->txtColor = COLOR_TEXT_0;
	sampleButton->label("-- MIDI --");
	sampleButton->redraw();
}


/* ------------------------------------------------------------------ */


void gMidiChannel::update() {

	if (ch->midiOut) {
		char tmp[32];
		sprintf(tmp, "-- MIDI (channel %d) --", ch->midiOutChan+1);
		sampleButton->copy_label(tmp);
	}
	else
		sampleButton->label("-- MIDI --");

	vol->value(ch->volume);
	mute->value(ch->mute);
	solo->value(ch->solo);
}


/* ------------------------------------------------------------------ */


int gMidiChannel::keyPress(int e) {
	return 1; // does nothing for MidiChannel
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gKeyboard::gKeyboard(int X, int Y, int W, int H, const char *L)
: Fl_Scroll(X, Y, W, H, L),
	bckspcPressed(false),
	endPressed(false),
	spacePressed(false)
{
	color(COLOR_BG_MAIN);
	type(Fl_Scroll::VERTICAL);
	scrollbar.color(COLOR_BG_0);
	scrollbar.selection_color(COLOR_BG_1);
	scrollbar.labelcolor(COLOR_BD_1);
	scrollbar.slider(G_BOX);

	gChannelsL  = new Fl_Group(x(), y(), (w()/2)-16, 0);
	gChannelsR  = new Fl_Group(gChannelsL->x()+gChannelsL->w()+32, y(), (w()/2)-16, 0);
	addChannelL = new gClick(gChannelsL->x(), gChannelsL->y()+gChannelsL->h(), gChannelsL->w(), 20, "Add new channel");
	addChannelR = new gClick(gChannelsR->x(), gChannelsR->y()+gChannelsR->h(), gChannelsR->w(), 20, "Add new channel");

	/* begin() - end() don't work well here, with sub-Fl_Group */

	add(addChannelL);
	add(addChannelR);
	add(gChannelsL);
	add(gChannelsR);

	gChannelsL->resizable(NULL);
	gChannelsR->resizable(NULL);

	addChannelL->callback(cb_addChannelL, (void*) this);
	addChannelR->callback(cb_addChannelR, (void*) this);
}


/* ------------------------------------------------------------------ */


int gKeyboard::openChanTypeMenu() {

	Fl_Menu_Item rclick_menu[] = {
		{"Sample channel"},
		{"MIDI channel"},
		{0}
	};

	Fl_Menu_Button *b = new Fl_Menu_Button(0, 0, 100, 50);
	b->box(G_BOX);
	b->textsize(11);
	b->textcolor(COLOR_TEXT_0);
	b->color(COLOR_BG_0);

	const Fl_Menu_Item *m = rclick_menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, b);
	if (!m) return 0;

	if (strcmp(m->label(), "Sample channel") == 0)
		return CHANNEL_SAMPLE;
	if (strcmp(m->label(), "MIDI channel") == 0)
		return CHANNEL_MIDI;
	return 0;
}


/* ------------------------------------------------------------------ */


void gKeyboard::fixRightColumn() {
	if (!hasScrollbar())
		gChannelsR->position(gChannelsL->x()+gChannelsL->w()+32, gChannelsR->y());
	else
		gChannelsR->position(gChannelsL->x()+gChannelsL->w()+8, gChannelsR->y());
	addChannelR->position(gChannelsR->x(), addChannelR->y());
}


/* ------------------------------------------------------------------ */


void gKeyboard::freeChannel(gChannel *gch) {
	gch->reset();
}


/* ------------------------------------------------------------------ */


void gKeyboard::deleteChannel(gChannel *gch) {
	Fl::lock();
	gch->hide();
	gChannelsR->remove(gch);
	gChannelsL->remove(gch);
	delete gch;
	//ch->guiChannel = NULL; old stuff: still required?
	Fl::unlock();
	fixRightColumn();
}


/* ------------------------------------------------------------------ */


void gKeyboard::updateChannel(gChannel *gch) {
	gch->update();
}


/* ------------------------------------------------------------------ */


void gKeyboard::updateChannels(char side) {

	Fl_Group *group;
	gClick   *add;

	if (side == 0)	{
		group = gChannelsL;
		add   = addChannelL;
	}
	else {
		group = gChannelsR;
		add   = addChannelR;
	}

	//gLog("[keyboard::updateChannels] side %d has %d widgets\n", side, group->children());

	for (int i=0; i<group->children(); i++) {
		gChannel *gch = (gChannel*) group->child(i);
		gch->position(gch->x(), group->y()+(i*24));
	}
	group->size(group->w(), group->children()*24);
	add->position(add->x(), group->y()+group->h());

	redraw();
}


/* ------------------------------------------------------------------ */


void gKeyboard::cb_addChannelL(Fl_Widget *v, void *p) { ((gKeyboard*)p)->__cb_addChannelL(); }
void gKeyboard::cb_addChannelR(Fl_Widget *v, void *p) { ((gKeyboard*)p)->__cb_addChannelR(); }


/* ------------------------------------------------------------------ */


gChannel *gKeyboard::addChannel(char side, Channel *ch) {
	Fl_Group *group;
	gClick   *add;

	if (side == 0) {
		group = gChannelsL;
		add   = addChannelL;
	}
	else {
		group = gChannelsR;
		add   = addChannelR;
	}

	gChannel *gch = NULL;

	if (ch->type == CHANNEL_SAMPLE)
		gch = (gSampleChannel*) new gSampleChannel(
				group->x(),
				group->y() + group->children() * 24,
				group->w(),
				20,
				(SampleChannel*) ch);
	else
		gch = (gMidiChannel*) new gMidiChannel(
				group->x(),
				group->y() + group->children() * 24,
				group->w(),
				20,
				(MidiChannel*) ch);

	group->add(gch);
	group->size(group->w(), group->children() * 24);
	add->position(group->x(), group->y()+group->h());
	fixRightColumn();
	redraw();

	return gch;
}


/* ------------------------------------------------------------------ */


bool gKeyboard::hasScrollbar() {
	if (24 * (gChannelsL->children()) > h())
		return true;
	if (24 * (gChannelsR->children()) > h())
		return true;
	return false;
}


/* ------------------------------------------------------------------ */


void gKeyboard::__cb_addChannelL() {
	int type = openChanTypeMenu();
	if (type)
		glue_addChannel(0, type);
}


void gKeyboard::__cb_addChannelR() {
	int type = openChanTypeMenu();
	if (type)
		glue_addChannel(1, type);
}


/* ------------------------------------------------------------------ */


int gKeyboard::handle(int e) {
	int ret = Fl_Group::handle(e);  // assume the buttons won't handle the Keyboard events
	switch (e) {
		case FL_FOCUS:
		case FL_UNFOCUS: {
			ret = 1;                	// enables receiving Keyboard events
			break;
		}
		case FL_SHORTCUT:           // in case widget that isn't ours has focus
		case FL_KEYDOWN:            // Keyboard key pushed
		case FL_KEYUP: {            // Keyboard key released

			/* rewind session. Avoid retrigs */

			if (e == FL_KEYDOWN) {
				if (Fl::event_key() == FL_BackSpace && !bckspcPressed) {
					bckspcPressed = true;
					glue_rewindSeq();
					ret = 1;
					break;
				}
				else if (Fl::event_key() == FL_End && !endPressed) {
					endPressed = true;
					glue_startStopInputRec(false);  // update gui
					ret = 1;
					break;
				}
				else if (Fl::event_key() == FL_Enter && !enterPressed) {
					enterPressed = true;
					glue_startStopActionRec();
					ret = 1;
					break;
				}
				else if (Fl::event_key() == ' ' && !spacePressed) {
					spacePressed = true;
					G_Mixer.running ? glue_stopSeq() : glue_startSeq();
					ret = 1;
					break;
				}
			}
			else if (e == FL_KEYUP) {
				if (Fl::event_key() == FL_BackSpace)
					bckspcPressed = false;
				else if (Fl::event_key() == FL_End)
					endPressed = false;
				else if (Fl::event_key() == ' ')
					spacePressed = false;
				else if (Fl::event_key() == FL_Enter)
					enterPressed = false;
			}

			/* Walk button arrays, trying to match button's label with the Keyboard event.
			 * If found, set that button's value() based on up/down event,
			 * and invoke that button's callback() */

			for (int i=0; i<gChannelsL->children(); i++)
				ret &= ((gChannel*)gChannelsL->child(i))->keyPress(e);
			for (int i=0; i<gChannelsR->children(); i++)
				ret &= ((gChannel*)gChannelsR->child(i))->keyPress(e);
			break;
		}
	}
	return ret;
}


/* ------------------------------------------------------------------ */


void gKeyboard::clear() {
	Fl::lock();
	gChannelsL->clear();
	gChannelsR->clear();
	for (unsigned i=0; i<G_Mixer.channels.size; i++)
		G_Mixer.channels.at(i)->guiChannel = NULL;
	Fl::unlock();

	gChannelsR->size(gChannelsR->w(), 0);
	gChannelsL->size(gChannelsL->w(), 0);

	gChannelsL->resizable(NULL);
	gChannelsR->resizable(NULL);

	addChannelL->position(gChannelsL->x(), gChannelsL->y()+gChannelsL->h());
	addChannelR->position(gChannelsR->x(), gChannelsR->y()+gChannelsR->h());

	redraw();
}


/* ------------------------------------------------------------------ */


void gKeyboard::setChannelWithActions(gSampleChannel *gch) {
	if (gch->ch->hasActions)
		gch->addActionButton();
	else
		gch->delActionButton();
}

