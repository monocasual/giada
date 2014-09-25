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
#include "gd_keyGrabber.h"
#include "gd_midiGrabber.h"
#include "gd_midiOutputSetup.h"
#include "gd_actionEditor.h"
#include "gd_warnings.h"
#include "gd_mainWindow.h"
#include "gd_editor.h"
#include "const.h"
#include "mixer.h"
#include "wave.h"
#include "conf.h"
#include "patch.h"
#include "graphics.h"
#include "glue.h"
#include "recorder.h"
#include "pluginHost.h"
#include "channel.h"
#include "sampleChannel.h"
#include "midiChannel.h"
#include "log.h"

#ifdef WITH_VST
#include "gd_pluginList.h"
#endif


extern Mixer 		     G_Mixer;
extern Conf  		     G_Conf;
extern Patch 		     G_Patch;
extern gdMainWindow *mainWin;


gChannel::gChannel(int X, int Y, int W, int H)
 : Fl_Group(X, Y, W, H, NULL)
{
}


/* ------------------------------------------------------------------ */


int gChannel::getColumnIndex()
{
	return ((gColumn*)parent())->getIndex();
}


/* ------------------------------------------------------------------ */


gSampleChannel::gSampleChannel(int X, int Y, int W, int H, class SampleChannel *ch)
	: gChannel(X, Y, W, H), ch(ch)
{
	begin();

	button       = new gButton (x(), y(), 20, 20);
	status       = new gStatus (button->x()+button->w()+4, y(), 20, 20, ch);

#if defined(WITH_VST)
	sampleButton = new gClick  (status->x()+status->w()+4, y(), 192, 20, "-- no sample --");
#else
	sampleButton = new gClick  (status->x()+status->w()+4, y(), 216, 20, "-- no sample --");
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


void gSampleChannel::__cb_mute()
{
	glue_setMute(ch);
}


/* ------------------------------------------------------------------ */


void gSampleChannel::__cb_solo()
{
	solo->value() ? glue_setSoloOn(ch) : glue_setSoloOff(ch);
}


/* ------------------------------------------------------------------ */


void gSampleChannel::__cb_changeVol()
{
	glue_setChanVol(ch, vol->value());
}


/* ------------------------------------------------------------------ */


#ifdef WITH_VST
void gSampleChannel::__cb_openFxWindow()
{
	gu_openSubWindow(mainWin, new gdPluginList(PluginHost::CHANNEL, ch), WID_FX_LIST);
}
#endif

/* ------------------------------------------------------------------ */



void gSampleChannel::__cb_button()
{
	if (button->value())    // pushed
		glue_keyPress(ch, Fl::event_ctrl(), Fl::event_shift());
	else                    // released
		glue_keyRelease(ch, Fl::event_ctrl(), Fl::event_shift());
}


/* ------------------------------------------------------------------ */


void gSampleChannel::__cb_openMenu()
{
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


void gSampleChannel::__cb_readActions()
{
	ch->readActions ? glue_stopReadingRecs(ch) : glue_startReadingRecs(ch);
}


/* ------------------------------------------------------------------ */


void gSampleChannel::openBrowser(int type)
{
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


void gSampleChannel::refresh()
{
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


void gSampleChannel::reset()
{
	sampleButton->bgColor0 = COLOR_BG_0;
	sampleButton->bdColor  = COLOR_BD_0;
	sampleButton->txtColor = COLOR_TEXT_0;
	sampleButton->label("-- no sample --");
	delActionButton(true); // force==true, don't check, just remove it
 	sampleButton->redraw();
	status->redraw();
}


/* ------------------------------------------------------------------ */


void gSampleChannel::update()
{
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


int gSampleChannel::keyPress(int e)
{
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


void gSampleChannel::addActionButton()
{
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


void gSampleChannel::delActionButton(bool force)
{
	if (readActions == NULL)
		return;
	
	/* TODO - readActions check is useless here */
	
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
	sampleButton = new gClick (button->x()+button->w()+4, y(), 240, 20, "-- MIDI --");
#else
	sampleButton = new gClick (button->x()+button->w()+4, y(), 264, 20, "-- MIDI --");
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


void gMidiChannel::__cb_mute()
{
	glue_setMute(ch);
}


/* ------------------------------------------------------------------ */


void gMidiChannel::__cb_solo()
{
	solo->value() ? glue_setSoloOn(ch) : glue_setSoloOff(ch);
}


/* ------------------------------------------------------------------ */


void gMidiChannel::__cb_changeVol()
{
	glue_setChanVol(ch, vol->value());
}


/* ------------------------------------------------------------------ */


#ifdef WITH_VST
void gMidiChannel::__cb_openFxWindow()
{
	gu_openSubWindow(mainWin, new gdPluginList(PluginHost::CHANNEL, ch), WID_FX_LIST);
}
#endif

/* ------------------------------------------------------------------ */


void gMidiChannel::__cb_button()
{
	if (button->value())
		glue_keyPress(ch, Fl::event_ctrl(), Fl::event_shift());
}


/* ------------------------------------------------------------------ */


void gMidiChannel::__cb_openMenu()
{
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


void gMidiChannel::refresh()
{
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


void gMidiChannel::reset()
{
	sampleButton->bgColor0 = COLOR_BG_0;
	sampleButton->bdColor  = COLOR_BD_0;
	sampleButton->txtColor = COLOR_TEXT_0;
	sampleButton->label("-- MIDI --");
	sampleButton->redraw();
}


/* ------------------------------------------------------------------ */


void gMidiChannel::update()
{

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


int gMidiChannel::keyPress(int e)
{
	return 1; // does nothing for MidiChannel
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


int gKeyboard::indexColumn = 0;


/* ------------------------------------------------------------------ */


gKeyboard::gKeyboard(int X, int Y, int W, int H)
: Fl_Scroll    (X, Y, W, H),
	bckspcPressed(false),
	endPressed   (false),
	spacePressed (false),
	addColumnBtn (NULL)
{
	color(COLOR_BG_MAIN);
	type(Fl_Scroll::BOTH_ALWAYS);
	scrollbar.color(COLOR_BG_0);
	scrollbar.selection_color(COLOR_BG_1);
	scrollbar.labelcolor(COLOR_BD_1);
	scrollbar.slider(G_BOX);
	hscrollbar.color(COLOR_BG_0);
	hscrollbar.selection_color(COLOR_BG_1);
	hscrollbar.labelcolor(COLOR_BD_1);
	hscrollbar.slider(G_BOX);

	init();
}


/* ------------------------------------------------------------------ */


void gKeyboard::init()
{
	/* add 6 empty columns as init layout */
	
	__cb_addColumn();
	__cb_addColumn();
	__cb_addColumn();
	__cb_addColumn();
	__cb_addColumn();
	__cb_addColumn();
}


/* ------------------------------------------------------------------ */


void gKeyboard::freeChannel(gChannel *gch)
{
	gch->reset();
}


/* ------------------------------------------------------------------ */


void gKeyboard::deleteChannel(gChannel *gch)
{
	for (unsigned i=0; i<columns.size; i++) {
		int k = columns.at(i)->find(gch);
		if (k != columns.at(i)->children()) {
			columns.at(i)->deleteChannel(gch);
			return;
		}
	}
}


/* ------------------------------------------------------------------ */


void gKeyboard::updateChannel(gChannel *gch)
{
	gch->update();
}


/* ------------------------------------------------------------------ */


void gKeyboard::organizeColumns()
{
	/* if only one column exists don't cleanup: the initial column must
	 * stay here. */
	 
	if (columns.size == 1)
		return;
		
	/* otherwise delete all empty columns */
	/** FIXME - this for loop might not work correctly! */
		
	for (unsigned i=columns.size-1; i>=1; i--) {
		if (columns.at(i)->isEmpty()) {
			delete columns.at(i);
			columns.del(i);
		}
	}
	
	/* compact column, avoid empty spaces */
	
	for (unsigned i=1; i<columns.size; i++)
		columns.at(i)->position(columns.at(i-1)->x() + columns.at(i-1)->w() + 16, y());
	
	addColumnBtn->position(columns.last()->x() + columns.last()->w() + 16, y());

	redraw();
}


/* ------------------------------------------------------------------ */


void gKeyboard::cb_addColumn(Fl_Widget *v, void *p) { ((gKeyboard*)p)->__cb_addColumn(); }


/* ------------------------------------------------------------------ */


gChannel *gKeyboard::addChannel(int colIndex, Channel *ch, bool build)
{
	gColumn *col = NULL;
	
	if (build) {
		__cb_addColumn();
		col = columns.last();
		col->setIndex(colIndex);
	}
	else {
		for (unsigned i=0; i<columns.size; i++)
			if (columns.at(i)->getIndex() == colIndex) {
				col = columns.at(i);
				break;
			}
		if (!col) {
			gLog("[gKeyboard::addChannel] column not found!\n");
			return NULL;
		}	
		gLog("[ggKeyboard::addChannel] add to column with index = %d\n", col->getIndex());
	}
	return col->addChannel(ch);
}


/* ------------------------------------------------------------------ */


void gKeyboard::refreshColumns()
{
	for (unsigned i=0; i<columns.size; i++)
		columns.at(i)->refreshChannels();
}


/* ------------------------------------------------------------------ */


int gKeyboard::handle(int e) 
{
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

			for (unsigned i=0; i<columns.size; i++)
				for (int k=1; k<columns.at(i)->children(); k++)
					ret &= ((gChannel*)columns.at(i)->child(k))->keyPress(e);
			break;
		}
	}
	return ret;
}


/* ------------------------------------------------------------------ */


void gKeyboard::clear() 
{	
	Fl_Scroll::clear();
	columns.clear();
	indexColumn = 0;     // new columns will start from index=0
	init();
}


/* ------------------------------------------------------------------ */


void gKeyboard::setChannelWithActions(gSampleChannel *gch) 
{
	if (gch->ch->hasActions)
		gch->addActionButton();
	else
		gch->delActionButton();
}


/* ------------------------------------------------------------------ */


void gKeyboard::__cb_addColumn()
{
	int colx;
	int colxw;
	int colw = 360;
	if (columns.size == 0) {
		colx  = x() - xposition();  // mind the offset with xposition()
		colxw = colx + colw; 
		addColumnBtn = new gClick(colxw + 16, y(), 200, 20, "Add new column");
		addColumnBtn->callback(cb_addColumn, (void*) this);
		add(addColumnBtn);
	}
	else {
		gColumn *prev = columns.last();
		colx  = prev->x()+prev->w() + 16;
		colxw = colx + colw;
		addColumnBtn->position(colxw + 16, y());
	}
	gColumn *gc = new gColumn(colx, y(), colw, 2000, indexColumn);
	indexColumn++;
	add(gc);
	columns.add(gc);
	redraw();
	
	gLog("[gKeyboard] new column added (index = %d), total count=%d, addColumn=%d\n", gc->getIndex(), columns.size, addColumnBtn->x());
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gColumn::gColumn(int X, int Y, int W, int H, int index)
	: Fl_Group(X, Y, W, H), index(index)
{
	begin();
	addChannelBtn = new gClick(x(), y(), w(), 20, "Add new channel");
	end();
	
	resizable(NULL);

	addChannelBtn->callback(cb_addChannel, (void*)this);
}


/* ------------------------------------------------------------------ */


int gColumn::handle(int e)
{
	int ret = Fl_Group::handle(e);  // assume the buttons won't handle the Keyboard events
	switch (e) {
		case FL_DND_ENTER:         	// return(1) for these events to 'accept' dnd
		case FL_DND_DRAG: 
		case FL_DND_RELEASE: {
			gLog("dnd release\n");
			ret = 1;
			break;
		}
		case FL_PASTE: {            // handle actual drop (paste) operation
			gLog("Paste event: %s\n", Fl::event_text());
			ret = 1;
			break;			
		}
	}
	return ret;
}


/* ------------------------------------------------------------------ */


void gColumn::refreshChannels()
{
	for (int i=1; i<children(); i++)
		((gChannel*) child(i))->refresh();
}


/* ------------------------------------------------------------------ */


void gColumn::draw()
{
	fl_color(fl_rgb_color(27, 27, 27));
	fl_rectf(x(), y(), w(), h());
	Fl_Group::draw();
}


/* ------------------------------------------------------------------ */


void gColumn::cb_addChannel(Fl_Widget *v, void *p) { ((gColumn*)p)->__cb_addChannel(); }


/* ------------------------------------------------------------------ */


gChannel *gColumn::addChannel(class Channel *ch)
{
	gChannel *gch = NULL;
	
	if (ch->type == CHANNEL_SAMPLE)
		gch = (gSampleChannel*) new gSampleChannel(
				x(),
				y() + children() * 24,
				w(),
				20,
				(SampleChannel*) ch);
	else
		gch = (gMidiChannel*) new gMidiChannel(
				x(),
				y() + children() * 24,
				w(),
				20,
				(MidiChannel*) ch);	
	
	add(gch);
	size(w(), children() * 24); // TODO - add some space for drag n drop
	redraw();

	//~ ch->column = index;	
	return gch;
}


/* ------------------------------------------------------------------ */


void gColumn::deleteChannel(gChannel *gch)
{
	gch->hide();
	remove(gch);
	delete gch;
	
	/* reposition all other channels and resize this group */
	/** TODO
	 * reposition is useless when called by gColumn::clear(). Add a new
	 * parameter to skip the operation */
	 
	for (int i=0; i<children(); i++) {
		gch = (gChannel*) child(i);
		gch->position(gch->x(), y()+(i*24));
	}
	size(w(), children()*24);
	redraw();
}


/* ------------------------------------------------------------------ */


void gColumn::__cb_addChannel()
{
	gLog("[gColumn::__cb_addChannel] index = %d\n", index);
	int type = openTypeMenu();
	if (type)
		glue_addChannel(index, type);
}


/* ------------------------------------------------------------------ */


int gColumn::openTypeMenu()
{
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


void gColumn::clear(bool full)
{
	if (full)
		Fl_Group::clear();
	else {
		while (children() >= 2) {  // skip "add new channel" btn
			int i = children()-1;
			deleteChannel((gChannel*)child(i));
		}
	}
}
