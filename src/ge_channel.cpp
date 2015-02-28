/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_channel
 *
 * ---------------------------------------------------------------------
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
 * ------------------------------------------------------------------ */


#include "ge_channel.h"
#include "gd_mainWindow.h"
#include "gd_keyGrabber.h"
#include "gd_midiGrabber.h"
#include "gd_editor.h"
#include "gd_actionEditor.h"
#include "gd_warnings.h"
#include "gd_browser.h"
#include "gd_midiOutputSetup.h"
#include "gg_keyboard.h"
#include "pluginHost.h"
#include "mixer.h"
#include "conf.h"
#include "patch.h"
#include "graphics.h"
#include "channel.h"
#include "wave.h"
#include "sampleChannel.h"
#include "midiChannel.h"
#include "glue.h"
#include "gui_utils.h"

#ifdef WITH_VST
#include "gd_pluginList.h"
#endif


extern Mixer 		     G_Mixer;
extern Conf  		     G_Conf;
extern Patch 		     G_Patch;
extern gdMainWindow *mainWin;


gChannel::gChannel(int X, int Y, int W, int H, int type)
 : Fl_Group(X, Y, W, H, NULL), type(type)
{
}


/* -------------------------------------------------------------------------- */


int gChannel::getColumnIndex()
{
	return ((gColumn*)parent())->getIndex();
}


/* -------------------------------------------------------------------------- */


gSampleChannel::gSampleChannel(int X, int Y, int W, int H, class SampleChannel *ch)
	: gChannel(X, Y, W, H, CHANNEL_SAMPLE), ch(ch)
{
	begin();

#if defined(WITH_VST)
  int delta = 168; // (7 widgets * 20) + (7 paddings * 4)
#else
	int delta = 144; // (6 widgets * 20) + (6 paddings * 4)
#endif

	button       = new gButton (x(), y(), 20, 20);
	status       = new gStatus (button->x()+button->w()+4, y(), 20, 20, ch);
	sampleButton = new gChannelButton(status->x()+status->w()+4, y(), w() - delta, 20, "-- no sample --");
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

  resizable(sampleButton);

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


/* -------------------------------------------------------------------------- */


void gSampleChannel::cb_button      (Fl_Widget *v, void *p) { ((gSampleChannel*)p)->__cb_button(); }
void gSampleChannel::cb_mute        (Fl_Widget *v, void *p) { ((gSampleChannel*)p)->__cb_mute(); }
void gSampleChannel::cb_solo        (Fl_Widget *v, void *p) { ((gSampleChannel*)p)->__cb_solo(); }
void gSampleChannel::cb_openMenu    (Fl_Widget *v, void *p) { ((gSampleChannel*)p)->__cb_openMenu(); }
void gSampleChannel::cb_changeVol   (Fl_Widget *v, void *p) { ((gSampleChannel*)p)->__cb_changeVol(); }
void gSampleChannel::cb_readActions (Fl_Widget *v, void *p) { ((gSampleChannel*)p)->__cb_readActions(); }
#ifdef WITH_VST
void gSampleChannel::cb_openFxWindow(Fl_Widget *v, void *p) { ((gSampleChannel*)p)->__cb_openFxWindow(); }
#endif


/* -------------------------------------------------------------------------- */


void gSampleChannel::__cb_mute()
{
	glue_setMute(ch);
}


/* -------------------------------------------------------------------------- */


void gSampleChannel::__cb_solo()
{
	solo->value() ? glue_setSoloOn(ch) : glue_setSoloOff(ch);
}


/* -------------------------------------------------------------------------- */


void gSampleChannel::__cb_changeVol()
{
	glue_setChanVol(ch, vol->value());
}


/* -------------------------------------------------------------------------- */


#ifdef WITH_VST
void gSampleChannel::__cb_openFxWindow()
{
	gu_openSubWindow(mainWin, new gdPluginList(PluginHost::CHANNEL, ch), WID_FX_LIST);
}
#endif


/* -------------------------------------------------------------------------- */



void gSampleChannel::__cb_button()
{
	if (button->value())    // pushed
		glue_keyPress(ch, Fl::event_ctrl(), Fl::event_shift());
	else                    // released
		glue_keyRelease(ch, Fl::event_ctrl(), Fl::event_shift());
}


/* -------------------------------------------------------------------------- */


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
		{"Setup MIDI output..."},                   // 4
		{"Edit sample..."},                         // 5
		{"Edit actions..."},                        // 6
		{"Clear actions", 0, 0, 0, FL_SUBMENU},     // 7
			{"All"},                                  // 8
			{"Mute"},                                 // 9
			{"Volume"},                               // 10
			{"Start/Stop"},                           // 11
			{0},                                      // 12
		{"Free channel"},                           // 13
		{"Delete channel"},                         // 14
		{0}
	};

	if (ch->status & (STATUS_EMPTY | STATUS_MISSING)) {
		rclick_menu[1].deactivate();
		rclick_menu[5].deactivate();
		rclick_menu[13].deactivate();
	}

	/* no 'clear actions' if there are no actions */

	if (!ch->hasActions)
		rclick_menu[7].deactivate();

	/* no 'clear start/stop actions' for those channels in loop mode:
	 * they cannot have start/stop actions. */

	if (ch->mode & LOOP_ANY)
		rclick_menu[11].deactivate();

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
		gu_openSubWindow(mainWin, new gdMidiGrabberChannel(ch, GrabForInput), 0);
		return;
	}

	if (strcmp(m->label(), "Setup MIDI output...") == 0) {
		gu_openSubWindow(mainWin, new gdMidiGrabberChannel(ch, GrabForOutput), 0);
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
  if (!sampleButton->visible()) // sampleButton invisible? status too (see below)
    return;

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
		status->redraw(); // status invisible? sampleButton too (see below)
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
			sampleButton->label(ch->wave->name.c_str());
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

	readActions = new gClick(sampleButton->x() + sampleButton->w() + 4,
                           sampleButton->y(), 20, 20, "", readActionOff_xpm,
                           readActionOn_xpm);
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


void gSampleChannel::resize(int X, int Y, int W, int H)
{
  if (readActions)
    w() < BREAK_READ_ACTIONS ? readActions->hide() : readActions->show();

  w() < BREAK_MODE_BOX ? modeBox->hide() : modeBox->show();

  gChannel::resize(X, Y, W, H);
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


gMidiChannel::gMidiChannel(int X, int Y, int W, int H, class MidiChannel *ch)
	: gChannel(X, Y, W, H, CHANNEL_MIDI), ch(ch)
{
	begin();

#if defined(WITH_VST)
  int delta = 120; // (5 widgets * 20) + (5 paddings * 4)
#else
	int delta = 96; // (4 widgets * 20) + (4 paddings * 4)
#endif

	button       = new gButton (x(), y(), 20, 20);
	sampleButton = new gChannelButton(button->x()+button->w()+4, y(), w() - delta, 20, "-- MIDI --");
	mute         = new gClick (sampleButton->x()+sampleButton->w()+4, y(), 20, 20, "", muteOff_xpm, muteOn_xpm);
	solo         = new gClick (mute->x()+mute->w()+4, y(), 20, 20, "", soloOff_xpm, soloOn_xpm);
#if defined(WITH_VST)
	fx           = new gButton(solo->x()+solo->w()+4, y(), 20, 20, "", fxOff_xpm, fxOn_xpm);
	vol          = new gDial  (fx->x()+fx->w()+4, y(), 20, 20);
#else
	vol          = new gDial  (solo->x()+solo->w()+4, y(), 20, 20);
#endif

	end();

  resizable(sampleButton);

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
		gu_openSubWindow(mainWin, new gdMidiGrabberChannel(ch, GrabForInput), 0);
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


/* -------------------------------------------------------------------------- */


void gMidiChannel::resize(int X, int Y, int W, int H)
{
  sampleButton->w() < 20 ? sampleButton->hide() : sampleButton->show();
  gChannel::resize(X, Y, W, H);
}


/* -------------------------------------------------------------------------- */


int gMidiChannel::keyPress(int e)
{
	return 1; // does nothing for MidiChannel
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


gStatus::gStatus(int x, int y, int w, int h, SampleChannel *ch, const char *L)
: Fl_Box(x, y, w, h, L), ch(ch) {}

void gStatus::draw()
{
  fl_rect(x(), y(), w(), h(), COLOR_BD_0);              // reset border
  fl_rectf(x()+1, y()+1, w()-2, h()-2, COLOR_BG_0);     // reset background

  if (ch != NULL) {
    if (ch->status    & (STATUS_WAIT | STATUS_ENDING | REC_ENDING | REC_WAITING) ||
        ch->recStatus & (REC_WAITING | REC_ENDING))
    {
      fl_rect(x(), y(), w(), h(), COLOR_BD_1);
    }
    else
    if (ch->status == STATUS_PLAY)
      fl_rect(x(), y(), w(), h(), COLOR_BD_1);
    else
      fl_rectf(x()+1, y()+1, w()-2, h()-2, COLOR_BG_0);     // status empty


    if (G_Mixer.chanInput == ch)
      fl_rectf(x()+1, y()+1, w()-2, h()-2, COLOR_BG_3);     // take in progress
    else
    if (recorder::active && recorder::canRec(ch))
      fl_rectf(x()+1, y()+1, w()-2, h()-2, COLOR_BG_4);     // action record

    /* equation for the progress bar:
     * ((chanTracker - chanStart) * w()) / (chanEnd - chanStart). */

    int pos = ch->getPosition();
    if (pos == -1)
      pos = 0;
    else
      pos = (pos * (w()-1)) / (ch->end - ch->begin);
    fl_rectf(x()+1, y()+1, pos, h()-2, COLOR_BG_2);
  }
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


gModeBox::gModeBox(int x, int y, int w, int h, SampleChannel *ch, const char *L)
  : Fl_Menu_Button(x, y, w, h, L), ch(ch)
{
  box(G_BOX);
  textsize(11);
  textcolor(COLOR_TEXT_0);
  color(COLOR_BG_0);

  add("Loop . basic",      0, cb_change_chanmode, (void *)LOOP_BASIC);
  add("Loop . once",       0, cb_change_chanmode, (void *)LOOP_ONCE);
  add("Loop . once . bar", 0, cb_change_chanmode, (void *)LOOP_ONCE_BAR);
  add("Loop . repeat",     0, cb_change_chanmode, (void *)LOOP_REPEAT);
  add("Oneshot . basic",   0, cb_change_chanmode, (void *)SINGLE_BASIC);
  add("Oneshot . press",   0, cb_change_chanmode, (void *)SINGLE_PRESS);
  add("Oneshot . retrig",  0, cb_change_chanmode, (void *)SINGLE_RETRIG);
  add("Oneshot . endless", 0, cb_change_chanmode, (void *)SINGLE_ENDLESS);
}


/* -------------------------------------------------------------------------- */


void gModeBox::draw() {
  fl_rect(x(), y(), w(), h(), COLOR_BD_0);    // border
  switch (ch->mode) {
    case LOOP_BASIC:
      fl_draw_pixmap(loopBasic_xpm, x()+1, y()+1);
      break;
    case LOOP_ONCE:
      fl_draw_pixmap(loopOnce_xpm, x()+1, y()+1);
      break;
    case LOOP_ONCE_BAR:
      fl_draw_pixmap(loopOnceBar_xpm, x()+1, y()+1);
      break;
    case LOOP_REPEAT:
      fl_draw_pixmap(loopRepeat_xpm, x()+1, y()+1);
      break;
    case SINGLE_BASIC:
      fl_draw_pixmap(oneshotBasic_xpm, x()+1, y()+1);
      break;
    case SINGLE_PRESS:
      fl_draw_pixmap(oneshotPress_xpm, x()+1, y()+1);
      break;
    case SINGLE_RETRIG:
      fl_draw_pixmap(oneshotRetrig_xpm, x()+1, y()+1);
      break;
    case SINGLE_ENDLESS:
      fl_draw_pixmap(oneshotEndless_xpm, x()+1, y()+1);
      break;
  }
}


/* -------------------------------------------------------------------------- */


void gModeBox::cb_change_chanmode(Fl_Widget *v, void *p) { ((gModeBox*)v)->__cb_change_chanmode((intptr_t)p); }


/* -------------------------------------------------------------------------- */


void gModeBox::__cb_change_chanmode(int mode)
{
  ch->mode = mode;

  /* what to do when the channel is playing and you change the mode?
   * Nothing, since v0.5.3. Just refresh the action editor window, in
   * case it's open */

  gu_refreshActionEditor();
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


gChannelButton::gChannelButton(int x, int y, int w, int h, const char *L)
  : gClick(x, y, w, h, L) {}


/* -------------------------------------------------------------------------- */


int gChannelButton::handle(int e)
{
	int ret = gClick::handle(e);
	switch (e) {
		case FL_DND_ENTER:
		case FL_DND_DRAG:
		case FL_DND_RELEASE: {
			ret = 1;
			break;
		}
		case FL_PASTE: {
      gChannel *p = (gChannel*) parent();   // parent is g[Sample|Midi]Channel
      if (p->type == CHANNEL_SAMPLE) {
        SampleChannel *c = ((gSampleChannel*)p)->ch;
        int result = glue_loadChannel(c, gTrim(gStripFileUrl(Fl::event_text())).c_str());
  			if (result != SAMPLE_LOADED_OK)
  				mainWin->keyboard->printChannelMessage(result);
      }
      // else dnd on a MIDI channel, nothing to do so far (load MIDI tab in
      // the future)
			ret = 1;
			break;
		}
	}
	return ret;
}
