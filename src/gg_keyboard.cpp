/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gg_keyboard
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2012 Giovanni A. Zuliani | Monocasual
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


extern Mixer 		     G_Mixer;
extern Conf  		     G_Conf;
extern Patch 		     G_Patch;
extern gdMainWindow *mainWin;


Keyboard::Keyboard(int X,int Y,int W,int H,const char *L)
: Fl_Group(X,Y,W,H,L),
	bckspcPressed(false),
	endPressed(false),
	spacePressed(false)
{
	color(COLOR_BG_MAIN);

	/* read the config file */

	int arkeys[MAX_NUM_CHAN] = DEFAULT_KEY_ARRAY;
		for (int i=0; i<MAX_NUM_CHAN; i++)
			arkeys[i] = G_Conf.keys[i];

	for (unsigned i=0, _y=0, _x=0; i<MAX_NUM_CHAN; i++) {
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
	end();
}


/* ------------------------------------------------------------------ */


void Keyboard::cb_button      (Fl_Widget *v, void *p) { ((Keyboard*)p)->__cb_button((gButton*)v); }
void Keyboard::cb_openChanMenu(Fl_Widget *v, void *p) { ((Keyboard*)((gDial*)v)->parent())->__cb_openChanMenu((intptr_t)p); } // veramente bestiale...
void Keyboard::cb_change_vol  (Fl_Widget *v, void *p) { ((Keyboard*)((gDial*)v)->parent())->__cb_change_vol((intptr_t)p); }	 // veramente bestiale...
void Keyboard::cb_mute        (Fl_Widget *v, void *p) { ((Keyboard*)((gClick*)v)->parent())->__cb_mute((intptr_t)p); }	       // veramente bestiale...
void Keyboard::cb_readActions (Fl_Widget *v, void *p) { ((Keyboard*)((gClick*)v)->parent())->__cb_readActions((intptr_t)p); } // veramente bestiale...
#ifdef WITH_VST
void Keyboard::cb_openFxWindow(Fl_Widget *v, void *p) { mainWin->keyboard->__cb_openFxWindow((intptr_t)p); } /// perchè non semplificare anche gli altri?
#endif


/* ------------------------------------------------------------------ */


void Keyboard::__cb_button(gButton *gb) {
	if (gb->value())   // pushed
		glue_keyPress(gb->id, Fl::event_ctrl(), Fl::event_shift());
	else               // released
		glue_keyRelease(gb->id, Fl::event_ctrl(), Fl::event_shift());
}


/* ------------------------------------------------------------------ */


void Keyboard::__cb_openChanMenu(int chan) {

	/* if you're recording (actions or input) no menu is allowed; you can't
	 * do anything, especially deallocate the channel */

	if (G_Mixer.chanInput == chan || recorder::active)
		return;

	if (G_Mixer.chanStatus[chan] & (STATUS_EMPTY | STATUS_MISSING)) {
		char title[30];
		sprintf(title, "Browse Sample for Channel %d", chan+1);
		gWindow *childWin = new gdBrowser(title, G_Conf.samplePath, chan, BROWSER_LOAD_SAMPLE);
		gu_openSubWindow(mainWin, childWin,	WID_FILE_BROWSER);
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
		{0}
	};

	/* no 'clear actions' if there are no actions */

	if (!recorder::chanEvents[chan])
		rclick_menu[4].deactivate();

	/* no 'clear all actions' or 'clear start/stop actions' for those channels
	 * in loop mode: they can only have mute actions. */

	if (G_Mixer.chanMode[chan] & LOOP_ANY) {
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

		if (G_Mixer.chanStatus[chan] == STATUS_PLAY || (recorder::chanActive[chan] && G_Mixer.running))
			if (!gdConfirmWin("Warning", "This action may stop the channel: are you sure?"))
				return;

		char title[30];
		sprintf(title, "Browse Sample for Channel %d", chan+1);
		gWindow *childWin = new gdBrowser(title, G_Conf.samplePath, chan, BROWSER_LOAD_SAMPLE);
		gu_openSubWindow(mainWin, childWin,	WID_FILE_BROWSER);
		return;
	}

	if (strcmp(m->label(), "Edit sample...") == 0) {
		char title[26];
		sprintf(title, "Edit Sample in Channel %d", chan+1);
		gWindow *childWin = new gdEditor(title, chan);
		gu_openSubWindow(mainWin, childWin,	WID_SAMPLE_EDITOR);
		return;
	}

	if (strcmp(m->label(), "Export sample to file...") == 0) {
		char title[30];
		sprintf(title, "Save Sample in Channel %d", chan+1);
		gWindow *childWin = new gdBrowser(title, G_Conf.samplePath, chan, BROWSER_SAVE_SAMPLE);
		gu_openSubWindow(mainWin, childWin,	WID_FILE_BROWSER);
		return;
	}

	if (strcmp(m->label(), "Free channel") == 0) {
		if (G_Mixer.chanStatus[chan] == STATUS_PLAY) {
			if (!gdConfirmWin("Warning", "This action will stop the channel: are you sure?"))
				return;
		}
		else if (!gdConfirmWin("Warning", "Free channel: are you sure?"))
			return;

		glue_unloadChannel(chan);

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
		recorder::clearAction(chan, ACTION_MUTEON | ACTION_MUTEOFF);
		if (!recorder::chanEvents[chan])
			remActionButton(chan);

		/* TODO - set mute=false */

		gu_refreshActionEditor(); // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "Start/Stop") == 0) {
		if (!gdConfirmWin("Warning", "Clear all start/stop actions: are you sure?"))
			return;
		recorder::clearAction(chan, ACTION_KEYPRESS | ACTION_KEYREL | ACTION_KILLCHAN);
		if (!recorder::chanEvents[chan])
			remActionButton(chan);
		gu_refreshActionEditor();  // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "All") == 0) {
		if (!gdConfirmWin("Warning", "Clear all actions: are you sure?"))
			return;
		recorder::clearChan(chan);
		remActionButton(chan);
		gu_refreshActionEditor(); // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "Edit actions...") == 0) {
		gu_openSubWindow(mainWin, new gdActionEditor(chan),	WID_ACTION_EDITOR);
		return;
	}
}


/* ------------------------------------------------------------------ */


void Keyboard::__cb_change_vol(int chan) {
	G_Mixer.chanVolume[chan] = vol[chan]->value();
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

			for (int t=0; t<MAX_NUM_CHAN; t++) {

				if (e == FL_KEYDOWN && butts[t]->value())			 // key already pressed! skip it
					ret = 1;
				else {
					if (Fl::event_key() == butts[t]->key) {
						butts[t]->take_focus();                        // move focus to this button
						butts[t]->value((e == FL_KEYDOWN || e == FL_SHORTCUT) ? 1 : 0);      // change the button's state
						butts[t]->do_callback();                       // invoke the button's callback
						ret = 1;                               				 // indicate we handled it
					}
				}
			}
			break;
		}
		case FL_PUSH: {
			for (int t=0; t<MAX_NUM_CHAN; t++) {
				if (butts[t]->value()) {	      // if button ON do callback
					butts[t]->do_callback();
					break;
				}
			}
			break;
		}
	}
	return ret;
}


/* ------------------------------------------------------------------ */


void Keyboard::__cb_mute(int chan) {
	glue_writeMute(chan, true);
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



/* ------------------------------------------------------------------ */

#ifdef WITH_VST
void Keyboard::__cb_openFxWindow(int c) {
	gu_openSubWindow(mainWin, new gdPluginList(PluginHost::CHANNEL, c), WID_FX_LIST);
}
#endif
