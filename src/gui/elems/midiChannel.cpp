/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_midiChannel
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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
 * -------------------------------------------------------------------------- */


#include "../../core/mixer.h"
#include "../../core/conf.h"
#include "../../core/patch_DEPR_.h"
#include "../../core/graphics.h"
#include "../../core/channel.h"
#include "../../core/midiChannel.h"
#include "../../glue/channel.h"
#include "../../glue/main.h"
#include "../../glue/io.h"
#include "../../utils/gui.h"
#include "../dialogs/gd_mainWindow.h"
#include "../dialogs/gd_keyGrabber.h"
#include "../dialogs/gd_editor.h"
#include "../dialogs/gd_actionEditor.h"
#include "../dialogs/gd_warnings.h"
#include "../dialogs/gd_browser.h"
#include "../dialogs/gd_keyGrabber.h"
#include "../dialogs/gd_pluginList.h"
#include "../dialogs/midiIO/midiInputChannel.h"
#include "../dialogs/midiIO/midiOutputMidiCh.h"
#include "midiChannel.h"


extern Mixer 		     G_Mixer;
extern Conf  		     G_Conf;
extern Recorder			 G_Recorder;
extern Patch_DEPR_   G_Patch_DEPR_;
extern gdMainWindow *G_MainWin;


geMidiChannel::geMidiChannel(int X, int Y, int W, int H, MidiChannel *ch)
	: geChannel(X, Y, W, H, CHANNEL_MIDI, ch)
{
	begin();

#if defined(WITH_VST)
  int delta = 144; // (6 widgets * 20) + (6 paddings * 4)
#else
	int delta = 120; // (5 widgets * 20) + (5 paddings * 4)
#endif

	button     = new gButton(x(), y(), 20, 20, "", channelStop_xpm, channelPlay_xpm);
	arm        = new gClick(button->x()+button->w()+4, y(), 20, 20, "", armOff_xpm, armOn_xpm);
	mainButton = new geMidiChannelButton(arm->x()+arm->w()+4, y(), w() - delta, 20, "-- MIDI --");
	mute       = new gClick(mainButton->x()+mainButton->w()+4, y(), 20, 20, "", muteOff_xpm, muteOn_xpm);
	solo       = new gClick(mute->x()+mute->w()+4, y(), 20, 20, "", soloOff_xpm, soloOn_xpm);
#if defined(WITH_VST)
	fx         = new gFxButton(solo->x()+solo->w()+4, y(), 20, 20, fxOff_xpm, fxOn_xpm);
	vol        = new gDial(fx->x()+fx->w()+4, y(), 20, 20);
#else
	vol        = new gDial(solo->x()+solo->w()+4, y(), 20, 20);
#endif

	end();

  resizable(mainButton);

	update();

	button->callback(cb_button, (void*)this);
	button->when(FL_WHEN_CHANGED);   // do callback on keypress && on keyrelease

	arm->type(FL_TOGGLE_BUTTON);
	arm->callback(cb_arm, (void*)this);

#ifdef WITH_VST
	fx->callback(cb_openFxWindow, (void*)this);
#endif

	mute->type(FL_TOGGLE_BUTTON);
	mute->callback(cb_mute, (void*)this);

	solo->type(FL_TOGGLE_BUTTON);
	solo->callback(cb_solo, (void*)this);

	mainButton->callback(cb_openMenu, (void*)this);

	vol->callback(cb_changeVol, (void*)this);

	ch->guiChannel = this;
}


/* -------------------------------------------------------------------------- */


void geMidiChannel::cb_button      (Fl_Widget *v, void *p) { ((geMidiChannel*)p)->__cb_button(); }
void geMidiChannel::cb_openMenu    (Fl_Widget *v, void *p) { ((geMidiChannel*)p)->__cb_openMenu(); }


/* -------------------------------------------------------------------------- */


void geMidiChannel::__cb_button()
{
	if (button->value())
		glue_keyPress(ch, Fl::event_ctrl(), Fl::event_shift());
}


/* -------------------------------------------------------------------------- */


void geMidiChannel::__cb_openMenu()
{
	Fl_Menu_Item rclick_menu[] = {
		{"Edit actions..."},                        // 0
		{"Clear actions", 0, 0, 0, FL_SUBMENU},     // 1
			{"All"},                                  // 2
			{0},                                      // 3
		{"Setup keyboard input..."},                // 5
		{"Setup MIDI input..."},                    // 6
		{"Setup MIDI output..."},                   // 7
		{"Clone channel"},                          // 8
		{"Delete channel"},                         // 9
		{0}
	};

	/* no 'clear actions' if there are no actions */

	if (!ch->hasActions)
		rclick_menu[1].deactivate();

	Fl_Menu_Button *b = new Fl_Menu_Button(0, 0, 100, 50);
	b->box(G_BOX);
	b->textsize(GUI_FONT_SIZE_BASE);
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

	if (strcmp(m->label(), "Clone channel") == 0) {
		glue_cloneChannel(ch);
		return;
	}

	if (strcmp(m->label(), "Setup keyboard input...") == 0) {
		gu_openSubWindow(G_MainWin, new gdKeyGrabber(ch),	0);
		//new gdKeyGrabber(ch);
		return;
	}

	if (strcmp(m->label(), "All") == 0) {
		if (!gdConfirmWin("Warning", "Clear all actions: are you sure?"))
			return;
		G_Recorder.clearChan(ch->index);
    ch->hasActions = false;
		gu_refreshActionEditor(); // refresh a.editor window, it could be open
		return;
	}

	if (strcmp(m->label(), "Edit actions...") == 0) {
		gu_openSubWindow(G_MainWin, new gdActionEditor(ch),	WID_ACTION_EDITOR);
		return;
	}

	if (strcmp(m->label(), "Setup MIDI input...") == 0) {
		gu_openSubWindow(G_MainWin, new gdMidiInputChannel(ch), 0);
		return;
	}

	if (strcmp(m->label(), "Setup MIDI output...") == 0) {
		//gu_openSubWindow(G_MainWin, new gdMidiGrabberChannel(ch, GrabForOutput), 0);
		gu_openSubWindow(G_MainWin, new gdMidiOutputMidiCh((MidiChannel*) ch), 0);
		return;
	}
}


/* -------------------------------------------------------------------------- */


void geMidiChannel::refresh()
{
	setColorsByStatus(ch->status, ch->recStatus);
	mainButton->redraw();
}


/* -------------------------------------------------------------------------- */


void geMidiChannel::reset()
{
	mainButton->setDefaultMode("-- MIDI --");
	mainButton->redraw();
}


/* -------------------------------------------------------------------------- */


void geMidiChannel::update()
{
	if (((MidiChannel*) ch)->midiOut) {
		char tmp[32];
		sprintf(tmp, "-- MIDI (channel %d) --", ((MidiChannel*) ch)->midiOutChan+1);
		mainButton->copy_label(tmp);
	}
	else
		mainButton->label("-- MIDI --");

	vol->value(ch->volume);
	mute->value(ch->mute);
	solo->value(ch->solo);

	mainButton->setKey(ch->key);

#ifdef WITH_VST
	fx->full = ch->plugins.size() > 0;
	fx->redraw();
#endif
}


/* -------------------------------------------------------------------------- */


void geMidiChannel::resize(int X, int Y, int W, int H)
{
  geChannel::resize(X, Y, W, H);

	arm->hide();
#ifdef WITH_VST
	fx->hide();
#endif

	if (w() > BREAK_ARM)
		arm->show();
#ifdef WITH_VST
	if (w() > BREAK_FX)
		fx->show();
#endif

	packWidgets();
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


geMidiChannelButton::geMidiChannelButton(int x, int y, int w, int h, const char *l)
	: geChannelButton(x, y, w, h, l) {}


/* -------------------------------------------------------------------------- */


int geMidiChannelButton::handle(int e)
{
	// MIDI drag-n-drop does nothing so far.
	return gClick::handle(e);
}
