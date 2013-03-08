/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gg_keyboard
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


extern Mixer 		     G_Mixer;
extern Conf  		     G_Conf;
extern Patch 		     G_Patch;
extern gdMainWindow *mainWin;


gChannel::gChannel(int X, int Y, int W, int H, const char* L, channel *ch)
: Fl_Group(X, Y, W, H, L), ch(ch)
{
	button = new gButton (x(), y(), 20, 20);
	status = new gStatus (button->x()+button->w()+4, y(), 20, 20, NULL);
#if defined(WITH_VST)
	sampleButton = new gClick  (status->x()+status->w()+4, y(), 239, 20, "-- no sample --");
	mute         = new gClick  (sampleButton->x()+sampleButton->w()+4, y(), 20, 20, "", muteOff_xpm, muteOn_xpm);
	fx           = new gButton (mute->x()+mute->w()+4, y(), 20, 20, "", fxOff_xpm, fxOn_xpm);
	vol          = new gDial   (fx->x()+fx->w()+4, y(), 20, 20);
#else
	sampleButton = new gClick  (status->x()+status->w()+4, y(), 263, 20, "-- no sample --");
	mute         = new gClick  (sampleButton->x()+sampleButton->w()+4, y(), 20,  20, "", muteOff_xpm, muteOn_xpm);
	vol          = new gDial   (mute->x()+mute->w()+4, y(), 20, 20);
#endif
	end();

	if (ch->wave)
		gu_trim_label(ch->wave->name.c_str(), 28, sampleButton);

	modeBox    = new gModeBox(vol->x()+vol->w()+4, y(), 20, 20);
	readAction = NULL; // no rec button at start

	button->callback(cb_button, (void*)this);
	/*
	char buf[2]; sprintf(buf, "%c", arkeys[i]);
	button->copy_label(buf);
	button->key = arkeys[i];
	button->id  = i;
	*/

#ifdef WITH_VST
	fx->callback(cb_openFxWindow, (void*)this);
#endif

	mute->type(FL_TOGGLE_BUTTON);
	mute->callback(cb_mute, (void*)this);
	sampleButton->callback(cb_openChanMenu, (void*)this);
	vol->callback(cb_change_vol, (void*)this);
}


/* ------------------------------------------------------------------ */


void gChannel::cb_button      (Fl_Widget *v, void *p) { ((gChannel*)p)->__cb_button(); }
void gChannel::cb_mute        (Fl_Widget *v, void *p) { ((gChannel*)p)->__cb_mute(); }
void gChannel::cb_openChanMenu(Fl_Widget *v, void *p) { ((gChannel*)p)->__cb_openChanMenu(); }
void gChannel::cb_change_vol  (Fl_Widget *v, void *p) { ((gChannel*)p)->__cb_change_vol(); }
#ifdef WITH_VST
void gChannel::cb_openFxWindow(Fl_Widget *v, void *p) { ((gChannel*)p)->__cb_openFxWindow(); }
#endif


/* ------------------------------------------------------------------ */


void gChannel::__cb_button() {
	if (button->value())    // pushed
		glue_keyPress(ch, Fl::event_ctrl(), Fl::event_shift());
	else                    // released
		glue_keyRelease(ch, Fl::event_ctrl(), Fl::event_shift());
}


/* ------------------------------------------------------------------ */


#ifdef WITH_VST
void gChannel::__cb_openFxWindow() {
	gu_openSubWindow(mainWin, new gdPluginList(PluginHost::CHANNEL, ch), WID_FX_LIST);
}
#endif


/* ------------------------------------------------------------------ */


void gChannel::reset() {
	sampleButton->bgColor0 = COLOR_BG_0;
	sampleButton->bdColor  = COLOR_BD_0;
	sampleButton->txtColor = COLOR_TEXT_0;
	sampleButton->label("-- no sample --");
	///remActionButton();
	sampleButton->redraw();
	status->redraw();
}


/* ------------------------------------------------------------------ */


void gChannel::__cb_mute() {
	glue_setMute(ch);
}


/* ------------------------------------------------------------------ */


void gChannel::__cb_openChanMenu() {

	if (ch == NULL) {
		openBrowser(BROWSER_LOAD_SAMPLE);
		return;
	}

	/* if you're recording (actions or input) no menu is allowed; you can't
	 * do anything, especially deallocate the channel */

	if (G_Mixer.chanInput == ch->index || recorder::active)
		return;

	if (ch->status & (STATUS_EMPTY | STATUS_MISSING)) {
		openBrowser(BROWSER_LOAD_SAMPLE);
		return;
	}

	/* the following is a trash workaround for a FLTK menu. We need a gMenu
	 * widget asap */

	Fl_Menu_Item rclick_menu[] = {
		{"Load new sample..."},
		{"Export sample to file..."},
		{"Edit sample..."},
		{"Edit actions..."},
		{"Clear actions", 0, 0, 0, FL_SUBMENU},
			{"All"},
			{"Mute"},
			{"Start/Stop"},
			{0},
		{"Free channel"},
		{"Delete channel"},
		{0}
	};

	/* no 'clear actions' if there are no actions */

	if (!recorder::chanEvents[ch->index])
		rclick_menu[4].deactivate();

	/* no 'clear all actions' or 'clear start/stop actions' for those channels
	 * in loop mode: they can only have mute actions. */

	if (ch->mode & LOOP_ANY) {
		rclick_menu[5].deactivate();
		rclick_menu[7].deactivate();
	}

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

	if (strcmp(m->label(), "Edit sample...") == 0) {
		gu_openSubWindow(mainWin, new gdEditor("title", ch), WID_SAMPLE_EDITOR);
		return;
	}

	if (strcmp(m->label(), "Export sample to file...") == 0) {
		openBrowser(BROWSER_SAVE_SAMPLE);
		return;
	}

	if (strcmp(m->label(), "Delete channel") == 0) {
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
		if (!recorder::chanEvents[ch->index])
			((Keyboard*)parent())->remActionButton(ch->index);

		/* TODO - set mute=false */

		gu_refreshActionEditor(); // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "Start/Stop") == 0) {
		if (!gdConfirmWin("Warning", "Clear all start/stop actions: are you sure?"))
			return;
		recorder::clearAction(ch->index, ACTION_KEYPRESS | ACTION_KEYREL | ACTION_KILLCHAN);
		if (!recorder::chanEvents[ch->index])
			((Keyboard*)parent())->remActionButton(ch->index);
		gu_refreshActionEditor();  // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "All") == 0) {
		if (!gdConfirmWin("Warning", "Clear all actions: are you sure?"))
			return;
		recorder::clearChan(ch->index);
		((Keyboard*)parent())->remActionButton(ch->index);
		gu_refreshActionEditor(); // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "Edit actions...") == 0) {
		gu_openSubWindow(mainWin, new gdActionEditor(ch),	WID_ACTION_EDITOR);
		return;
	}
}


/* ------------------------------------------------------------------ */


void gChannel::openBrowser(int type) {
	const char *title = "";
	switch (type) {
		case BROWSER_LOAD_SAMPLE:
			title = "Browse Sample"; //sprintf(title, "Browse Sample", ch->index+1);
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


void gChannel::__cb_change_vol() {
	glue_setVolMainWin(ch, vol->value());
}


/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */
/* ------------------------------------------------------------------ */


Keyboard::Keyboard(int X, int Y, int W, int H, const char *L)
: Fl_Scroll(X, Y, W, H, L),
	bckspcPressed(false),
	endPressed(false),
	spacePressed(false)
{
	color(COLOR_BG_MAIN);
	type(VERTICAL);

	/* read the config file */
printf("[keyboard] children = %d\n", children());

	begin();
/**
	int arkeys[MAX_NUM_CHAN] = DEFAULT_KEY_ARRAY;
		for (int i=0; i<MAX_NUM_CHAN; i++)
			arkeys[i] = G_Conf.keys[i];

	for (unsigned i=0, _y=0, _x=0; i<G_Mixer.channels.size; i++) {
		if (i>=MAX_NUM_CHAN/2) {
			_y = 24*MAX_NUM_CHAN/2;
			_x = 411;
		}
		butts[i]        = new gButton (    x()+_x, (y()+i*24)-_y, 20,  20);
		status[i]       = new gStatus (24 +x()+_x, (y()+i*24)-_y, 20,  20, i);
#if defined(WITH_VST)
		sampleButton[i] = new gClick  (48 +x()+_x, (y()+i*24)-_y, 239, 20, "-- no sample --");
		mute[i]         = new gClick  (291+x()+_x, (y()+i*24)-_y, 20,  20, "", muteOff_xpm, muteOn_xpm);
		fx[i]           = new gButton (315+x()+_x, (y()+i*24)-_y, 20,  20, "", fxOff_xpm, fxOn_xpm);
#else
		sampleButton[i] = new gClick  (48 +x()+_x, (y()+i*24)-_y, 263, 20, "-- no sample --");
		mute[i]         = new gClick  (315+x()+_x, (y()+i*24)-_y, 20,  20, "", muteOff_xpm, muteOn_xpm);
#endif
		vol[i]          = new gDial   (339+x()+_x, (y()+i*24)-_y, 20,  20);
		modeBoxes[i]    = new gModeBox(363+x()+_x, (y()+i*24)-_y, 20,  20);
		readActions[i]  = NULL; // tutti vuoti all'inizio

		butts[i]->callback(cb_button, (void*)this);
		char buf[2]; sprintf(buf, "%c", arkeys[i]);
		butts[i]->copy_label(buf);
		butts[i]->key = arkeys[i];
		butts[i]->id  = i;

#ifdef WITH_VST
		fx[i]->id = i;
		fx[i]->callback(cb_openFxWindow, (void*)(intptr_t)i);
#endif

		mute[i]->type(FL_TOGGLE_BUTTON);
		mute[i]->callback(cb_mute, (void*)(intptr_t)i);

    sampleButton[i]->callback(cb_openChanMenu, (void*)(intptr_t)i);

		vol[i]->callback(cb_change_vol, (void*)(intptr_t)i);
	}
*/
	gChannels = new Fl_Group(x(), y(), w(), 40);
	gChannels->resizable(NULL);
	gChannels->box(FL_BORDER_BOX);

	updateChannels();

	end();
}


/* ------------------------------------------------------------------ */


void Keyboard::freeChannel(struct channel *ch) {
	for (int i=0; i<gChannels->children(); i++) {
		gChannel *gch = (gChannel*) gChannels->child(i);
		if (gch->ch == ch) {
			gch->reset();
			return;
		}
	}
}


/* ------------------------------------------------------------------ */


void Keyboard::deleteChannel(struct channel *ch) {
	for (int i=0; i<gChannels->children(); i++) {
		gChannel *gch = (gChannel*) gChannels->child(i);
		if (gch->ch == ch) {
			remove(gch);
			delete gch;
			return;
		}
	}
}


/* ------------------------------------------------------------------ */


void Keyboard::updateChannel(struct channel *ch) {
	for (int i=0; i<gChannels->children(); i++) {
		gChannel *gch = (gChannel*) gChannels->child(i);
		if (gch->ch == ch) {
			gu_trim_label(ch->wave->name.c_str(), 28, gch->sampleButton);
			return;
		}
	}
}


/* ------------------------------------------------------------------ */


gChannel *Keyboard::getChannel(struct channel *ch) {
	for (int i=0; i<gChannels->children(); i++) {
		gChannel *gch = (gChannel*) gChannels->child(i);
		if (gch->ch == ch)
			return gch;
	}
	return NULL;
}


/* ------------------------------------------------------------------ */


void Keyboard::updateChannels() {

	remove(addChannelL);
	remove(addChannelR);

	if (G_Mixer.channels.size == 0) {
		addChannelL = new gClick(x(), y() + h()/2, 359, 20, "Add new Left Channel");
		addChannelL->callback(cb_addChannelL, (void*) this);
		add(addChannelL);
	}
	else {
		gChannels->clear();
		gChannels->resizable(NULL);

		for (unsigned i=0; i<G_Mixer.channels.size; i++) {
			gChannel *gch = new gChannel(x(), i*24+y(), 359, 20, NULL, G_Mixer.channels.at(i));
			gChannels->add(gch);
			gChannels->size(w(), gChannels->children() * 24);
		}

		addChannelL = new gClick(x(), gChannels->y()+gChannels->h(), 359, 20, "Add new Left Channel");
		addChannelL->callback(cb_addChannelL, (void*) this);
		add(addChannelL);
	}
	redraw();
}


/* ------------------------------------------------------------------ */


void Keyboard::cb_readActions(Fl_Widget *v, void *p) { ((Keyboard*)((gClick*)v)->parent())->__cb_readActions((intptr_t)p); }
void Keyboard::cb_addChannelL(Fl_Widget *v, void *p) { ((Keyboard*)p)->__cb_addChannelL(); }
void Keyboard::cb_addChannelR(Fl_Widget *v, void *p) { ((Keyboard*)p)->__cb_addChannelR(); }


/* ------------------------------------------------------------------ */


void Keyboard::__cb_addChannelL() {
	channel  *ch  = G_Mixer.loadChannel(NULL, 0);
	gChannel *gch = new gChannel(x(), gChannels->y() + gChannels->children() * 24, 359, 20, NULL, ch);

	gChannels->add(gch);
	gChannels->size(w(), gChannels->children() * 24);
	addChannelL->position(x(), gChannels->y()+gChannels->h());
	redraw();
}

void Keyboard::__cb_addChannelR() {
	puts("add right channel");
}


/* ------------------------------------------------------------------ */


int Keyboard::handle(int e) {
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
					G_Mixer.chanInput == -1 ? glue_startInputRec() : glue_stopInputRec();
					ret = 1;
					break;
				}
				else if (Fl::event_key() == FL_Enter && !enterPressed) {
					enterPressed = true;
					recorder::active ? glue_stopRec() : glue_startRec();
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

			/* Walk button array, trying to match button's label with the Keyboard event.
			 * If found, set that button's value() based on up/down event,
			 * and invoke that button's callback() */

			for (int t=0; t<children(); t++) {
				gChannel *ch = (gChannel*) child(t);
				if (e == FL_KEYDOWN && ch->button->value())			 // key already pressed! skip it
					ret = 1;
				else {
					if (Fl::event_key() == ch->key) {
						ch->button->take_focus();                        // move focus to this button
						ch->button->value((e == FL_KEYDOWN || e == FL_SHORTCUT) ? 1 : 0);      // change the button's state
						ch->button->do_callback();                       // invoke the button's callback
						ret = 1;                               				 // indicate we handled it
					}
				}
			}
			break;
		}
		/*
		case FL_PUSH: {
			for (int t=0; t<children(); t++) {
				gChannel *ch = (gChannel*) child(t);
				if (ch->button->value()) {	      // if button ON do callback
					ch->button->do_callback();
					break;
				}
			}
			break;
		}
		*/
	}
	return ret;
}


/* ------------------------------------------------------------------ */


void Keyboard::__cb_readActions(int c) {
	recorder::chanActive[c] ? glue_stopReadingRecs(c) : glue_startReadingRecs(c);
}


/* ------------------------------------------------------------------ */


void Keyboard::addActionButton(int c, bool status) {

	/* quit if 'R' exists yet. */

	if (readActions[c] != NULL)
		return;

	sampleButton[c]->resize(
		sampleButton[c]->x(),
		sampleButton[c]->y(),
		sampleButton[c]->w()-24,
		sampleButton[c]->h());

	int _y = sampleButton[c]->y();
	int _x = sampleButton[c]->x() + sampleButton[c]->w() + 4;

	readActions[c] = new gClick(_x, _y, 20, 20, "", readActionOff_xpm, readActionOn_xpm);
	readActions[c]->type(FL_TOGGLE_BUTTON);
	readActions[c]->value(status);
	readActions[c]->callback(cb_readActions, (void*)(intptr_t)c);
	add(readActions[c]);

	/* hard redraw: there's no other way to avoid glitches when moving
	 * the 'R' button */
	/** FIXME - check with fltk 1.3.x */

	mainWin->redraw();
}


/* ------------------------------------------------------------------ */


void Keyboard::remActionButton(int c) {
	if (readActions[c] == NULL)
		return;

	remove(readActions[c]);		// delete from Keyboard group (FLTK)
	delete readActions[c];    // delete (C++)
	readActions[c] = NULL;

	sampleButton[c]->resize(
		sampleButton[c]->x(),
		sampleButton[c]->y(),
		sampleButton[c]->w()+24,
		sampleButton[c]->h());

	sampleButton[c]->redraw();
}
