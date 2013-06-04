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
#include "channel.h"
#include "gd_keyGrabber.h"


extern Mixer 		     G_Mixer;
extern Conf  		     G_Conf;
extern Patch 		     G_Patch;
extern gdMainWindow *mainWin;


gChannel::gChannel(int X, int Y, int W, int H, const char* L, channel *ch)
: Fl_Group(X, Y, W, H, L), ch(ch)
{
	begin();
	button = new gButton (x(), y(), 20, 20);

	if (ch->type == CHANNEL_SAMPLE)
		status = new gStatus (button->x()+button->w()+4, y(), 20, 20, ch);

	const char *sampleLabel;
	int sampleButton_x;
	int sampleButton_w;

	if (ch->type == CHANNEL_SAMPLE) {
		sampleLabel = "-- no sample --";
		sampleButton_x = status->x()+status->w()+4;
#if defined(WITH_VST)
		sampleButton_w = 213;
#else
		sampleButton_w = 237;
#endif
	}
	else {
		sampleLabel = "-- MIDI --";
		sampleButton_x = button->x()+button->w()+4;
#if defined(WITH_VST)
		sampleButton_w = 237;
#else
		sampleButton_w = 261;
#endif
	}

#if defined(WITH_VST)
	sampleButton = new gClick  (sampleButton_x, y(), sampleButton_w, 20, sampleLabel);
	mute         = new gClick  (sampleButton->x()+sampleButton->w()+4, y(), 20, 20, "", muteOff_xpm, muteOn_xpm);
	solo         = new gClick  (mute->x()+mute->w()+4, y(), 20, 20, "", soloOff_xpm, soloOn_xpm);
	fx           = new gButton (solo->x()+solo->w()+4, y(), 20, 20, "", fxOff_xpm, fxOn_xpm);
	vol          = new gDial   (fx->x()+fx->w()+4, y(), 20, 20);
#else
	sampleButton = new gClick  (sampleButton_x, y(), sampleButton_w, 20, sampleLabel);
	mute         = new gClick  (sampleButton->x()+sampleButton->w()+4, y(), 20,  20, "", muteOff_xpm, muteOn_xpm);
	solo         = new gClick  (mute->x()+mute->w()+4, y(), 20, 20, "", soloOff_xpm, soloOn_xpm);
	vol          = new gDial   (solo->x()+solo->w()+4, y(), 20, 20);
#endif
	modeBox      = new gModeBox(vol->x()+vol->w()+4, y(), 20, 20, ch);
	readActions  = NULL; // no rec button at start
	end();

	if (ch->wave)
		gu_trim_label(ch->wave->name.c_str(), 28, sampleButton);

	button->callback(cb_button, (void*)this);
	button->when(FL_WHEN_CHANGED);   // do callback on keypress && on keyrelease

	char buf[2]; sprintf(buf, "%c", ch->key);
	button->copy_label(buf);

#ifdef WITH_VST
	fx->callback(cb_openFxWindow, (void*)this);
#endif

	mute->type(FL_TOGGLE_BUTTON);
	mute->callback(cb_mute, (void*)this);

	solo->type(FL_TOGGLE_BUTTON);
	solo->callback(cb_solo, (void*)this);

	sampleButton->callback(cb_openChanMenu, (void*)this);
	vol->callback(cb_change_vol, (void*)this);

	ch->guiChannel = this;
}


/* ------------------------------------------------------------------ */


void gChannel::cb_button      (Fl_Widget *v, void *p) { ((gChannel*)p)->__cb_button(); }
void gChannel::cb_mute        (Fl_Widget *v, void *p) { ((gChannel*)p)->__cb_mute(); }
void gChannel::cb_solo        (Fl_Widget *v, void *p) { ((gChannel*)p)->__cb_solo(); }
void gChannel::cb_openChanMenu(Fl_Widget *v, void *p) { ((gChannel*)p)->__cb_openChanMenu(); }
void gChannel::cb_change_vol  (Fl_Widget *v, void *p) { ((gChannel*)p)->__cb_change_vol(); }
void gChannel::cb_readActions (Fl_Widget *v, void *p) { ((gChannel*)p)->__cb_readActions(); }
#ifdef WITH_VST
void gChannel::cb_openFxWindow(Fl_Widget *v, void *p) { ((gChannel*)p)->__cb_openFxWindow(); }
#endif


/* ------------------------------------------------------------------ */

int gChannel::keypress(int e) {
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

	if (ch->type == CHANNEL_SAMPLE)
		sampleButton->label("-- no sample --");
	else
		sampleButton->label("-- MIDI --");

	remActionButton();
	sampleButton->redraw();

	if (ch->type == CHANNEL_SAMPLE)
		status->redraw();
}


/* ------------------------------------------------------------------ */


void gChannel::__cb_mute() {
	glue_setMute(ch);
}


/* ------------------------------------------------------------------ */


void gChannel::__cb_solo() {
	solo->value() ? glue_setSoloOn(ch) : glue_setSoloOff(ch);
}


/* ------------------------------------------------------------------ */


void gChannel::__cb_openChanMenu() {

	/* if you're recording (actions or input) no menu is allowed; you can't
	 * do anything, especially deallocate the channel */

	if (G_Mixer.chanInput == ch || recorder::active)
		return;

	/* the following is a trash workaround for a FLTK menu. We need a gMenu
	 * widget asap */

	Fl_Menu_Item rclick_menu[] = {
		{"Load new sample..."},                     // 0
		{"Export sample to file..."},               // 1
		{"Set key..."},                             // 2
		{"Edit sample..."},                         // 3
		{"Edit actions..."},                        // 4
		{"Clear actions", 0, 0, 0, FL_SUBMENU},     // 5
			{"All"},                                  // 6
			{"Mute"},                                 // 7
			{"Volume"},                               // 8
			{"Start/Stop"},                           // 9
			{0},                                      // 10
		{"Free channel"},                           // 11
		{"Delete channel"},                         // 12
		{0}
	};

	if (ch->type == CHANNEL_MIDI) {
		rclick_menu[0].hide();
		rclick_menu[1].hide();
		rclick_menu[2].hide();
		rclick_menu[3].hide();
		rclick_menu[7].hide();
		rclick_menu[8].hide();
		rclick_menu[9].hide();
		rclick_menu[11].hide();
	}

	if (ch->status & (STATUS_EMPTY | STATUS_MISSING)) {
		rclick_menu[1].deactivate();
		rclick_menu[3].deactivate();
		if (ch->type == CHANNEL_SAMPLE)
			rclick_menu[4].deactivate();
		rclick_menu[11].deactivate();
	}

	/* no 'clear actions' if there are no actions */

	if (!ch->hasActions)
		rclick_menu[5].deactivate();

	/* no 'clear start/stop actions' for those channels in loop mode:
	 * they cannot have start/stop actions. */

	if (ch->mode & LOOP_ANY)
		rclick_menu[9].deactivate();

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

	if (strcmp(m->label(), "Set key...") == 0) {
		new gdKeyGrabber(ch);
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
		if (ch->wave != NULL && !gdConfirmWin("Warning", "Delete channel: are you sure?"))
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
			remActionButton();

		/* TODO - set mute=false */

		gu_refreshActionEditor(); // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "Start/Stop") == 0) {
		if (!gdConfirmWin("Warning", "Clear all start/stop actions: are you sure?"))
			return;
		recorder::clearAction(ch->index, ACTION_KEYPRESS | ACTION_KEYREL | ACTION_KILLCHAN);
		if (!ch->hasActions)
			remActionButton();
		gu_refreshActionEditor();  // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "Volume") == 0) {
		if (!gdConfirmWin("Warning", "Clear all volume actions: are you sure?"))
			return;
		recorder::clearAction(ch->index, ACTION_VOLUME);
		if (!ch->hasActions)
			remActionButton();
		gu_refreshActionEditor();  // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "All") == 0) {
		if (!gdConfirmWin("Warning", "Clear all actions: are you sure?"))
			return;
		recorder::clearChan(ch->index);
		remActionButton();
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


void gChannel::__cb_change_vol() {
	glue_setVolMainWin(ch, vol->value());
}


/* ------------------------------------------------------------------ */


void gChannel::addActionButton(bool status) {

	/* quit if 'R' exists yet. */

	if (readActions != NULL)
		return;

	sampleButton->size(sampleButton->w()-24, sampleButton->h());

	redraw();

	readActions = new gClick(sampleButton->x() + sampleButton->w() + 4, sampleButton->y(), 20, 20, "", readActionOff_xpm, readActionOn_xpm);
	readActions->type(FL_TOGGLE_BUTTON);
	readActions->value(status);
	readActions->callback(cb_readActions, (void*)this);
	add(readActions);

	/* hard redraw: there's no other way to avoid glitches when moving
	 * the 'R' button */

	mainWin->keyboard->redraw();
}


/* ------------------------------------------------------------------ */


void gChannel::remActionButton() {
	if (readActions == NULL)
		return;

	remove(readActions);		// delete from Keyboard group (FLTK)
	//delete readActions[c];  // delete (C++)
	readActions = NULL;

	sampleButton->size(sampleButton->w()+24, sampleButton->h());
	sampleButton->redraw();
}


/* ------------------------------------------------------------------ */


void gChannel::__cb_readActions() {
	ch->readActions ? glue_stopReadingRecs(ch) : glue_startReadingRecs(ch);
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
	type(Fl_Scroll::VERTICAL);
	scrollbar.color(COLOR_BG_0);
	scrollbar.selection_color(COLOR_BG_1);
	scrollbar.labelcolor(COLOR_BD_1);
	scrollbar.slider(G_BOX);

	gChannelsL  = new Fl_Group(x(), y(), (w()/2)-16, 0);
	gChannelsR  = new Fl_Group(gChannelsL->x()+gChannelsL->w()+32, y(), (w()/2)-16, 0);
	addChannelL = new gClick(gChannelsL->x(), gChannelsL->y()+gChannelsL->h(), gChannelsL->w(), 20, "Add new Left Channel");
	addChannelR = new gClick(gChannelsR->x(), gChannelsR->y()+gChannelsR->h(), gChannelsR->w(), 20, "Add new Right Channel");

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


int Keyboard::openChanTypeMenu() {

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


void Keyboard::fixRightColumn() {
	if (!hasScrollbar())
		gChannelsR->position(gChannelsL->x()+gChannelsL->w()+32, gChannelsR->y());
	else
		gChannelsR->position(gChannelsL->x()+gChannelsL->w()+8, gChannelsR->y());
	addChannelR->position(gChannelsR->x(), addChannelR->y());
}


/* ------------------------------------------------------------------ */


void Keyboard::freeChannel(struct channel *ch) {
	ch->guiChannel->reset();
}


/* ------------------------------------------------------------------ */


void Keyboard::deleteChannel(struct channel *ch) {
	Fl::lock();
	ch->guiChannel->hide();
	gChannelsR->remove(ch->guiChannel);
	gChannelsL->remove(ch->guiChannel);
	delete ch->guiChannel;
	ch->guiChannel = NULL;
	Fl::unlock();
	fixRightColumn();
}


/* ------------------------------------------------------------------ */


void Keyboard::updateChannel(struct channel *ch) {
	gu_trim_label(ch->wave->name.c_str(), 28, ch->guiChannel->sampleButton);
}


/* ------------------------------------------------------------------ */


void Keyboard::updateChannels(char side) {

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

	//printf("[keyboard::updateChannels] side %d has %d widgets\n", side, group->children());

	for (int i=0; i<group->children(); i++) {
		gChannel *gch = (gChannel*) group->child(i);
		gch->position(gch->x(), group->y()+(i*24));
	}
	group->size(group->w(), group->children()*24);
	add->position(add->x(), group->y()+group->h());

	redraw();
}


/* ------------------------------------------------------------------ */


void Keyboard::cb_addChannelL(Fl_Widget *v, void *p) { ((Keyboard*)p)->__cb_addChannelL(); }
void Keyboard::cb_addChannelR(Fl_Widget *v, void *p) { ((Keyboard*)p)->__cb_addChannelR(); }


/* ------------------------------------------------------------------ */


gChannel *Keyboard::addChannel(char side, channel *ch) {
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

	gChannel *gch = new gChannel(group->x(), group->y() + group->children() * 24, group->w(), 20, NULL, ch);

	group->add(gch);
	group->size(group->w(), group->children() * 24);
	add->position(group->x(), group->y()+group->h());
	fixRightColumn();
	redraw();

	return gch;
}


/* ------------------------------------------------------------------ */


bool Keyboard::hasScrollbar() {
	if (24 * (gChannelsL->children()) > h())
		return true;
	if (24 * (gChannelsR->children()) > h())
		return true;
	return false;
}


/* ------------------------------------------------------------------ */


void Keyboard::__cb_addChannelL() {
	int type = openChanTypeMenu();
	if (type)
		glue_addChannel(0, type);
}


void Keyboard::__cb_addChannelR() {
	int type = openChanTypeMenu();
	if (type)
		glue_addChannel(1, type);
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
					G_Mixer.chanInput == NULL ? glue_startInputRec() : glue_stopInputRec();
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

			/* Walk button arrays, trying to match button's label with the Keyboard event.
			 * If found, set that button's value() based on up/down event,
			 * and invoke that button's callback() */

			for (int i=0; i<gChannelsL->children(); i++)
				ret &= ((gChannel*)gChannelsL->child(i))->keypress(e);
			for (int i=0; i<gChannelsR->children(); i++)
				ret &= ((gChannel*)gChannelsR->child(i))->keypress(e);
			break;
		}
	}
	return ret;
}


/* ------------------------------------------------------------------ */


void Keyboard::clear() {
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


void Keyboard::setChannelWithActions(channel *ch) {
	if (ch->hasActions) {
		ch->readActions = true;   /// <---- move this to glue_stopRec
		ch->guiChannel->addActionButton(true); // true = button on
	}
	else {
		ch->readActions = false;  /// <---- move this to glue_stopRec
		ch->guiChannel->remActionButton();
	}
}

