/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_midiChannel
 *
 * -----------------------------------------------------------------------------
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
 * -------------------------------------------------------------------------- */


#include "../../core/pluginHost.h"
#include "../../core/mixer.h"
#include "../../core/conf.h"
#include "../../core/patch_DEPR_.h"
#include "../../core/graphics.h"
#include "../../core/channel.h"
#include "../../core/wave.h"
#include "../../core/sampleChannel.h"
#include "../../core/midiChannel.h"
#include "../../glue/channel.h"
#include "../../glue/glue.h"
#include "../../utils/gui_utils.h"
#include "../dialogs/gd_mainWindow.h"
#include "../dialogs/gd_keyGrabber.h"
#include "../dialogs/gd_midiInput.h"
#include "../dialogs/gd_editor.h"
#include "../dialogs/gd_actionEditor.h"
#include "../dialogs/gd_warnings.h"
#include "../dialogs/gd_browser.h"
#include "../dialogs/gd_keyGrabber.h"
#include "../dialogs/gd_midiOutput.h"
#include "ge_keyboard.h"
#include "ge_midiChannel.h"
#include "ge_channel.h"
#include "ge_sampleChannel.h"

#ifdef WITH_VST
#include "../dialogs/gd_pluginList.h"
#endif


extern Mixer 		     G_Mixer;
extern Conf  		     G_Conf;
extern Patch_DEPR_   G_Patch_DEPR_;
extern gdMainWindow *mainWin;


gMidiChannel::gMidiChannel(int X, int Y, int W, int H, class MidiChannel *ch)
	: gChannel(X, Y, W, H, CHANNEL_MIDI), ch(ch)
{
	begin();

#if defined(WITH_VST)
  int delta = 120; // (5 widgets * 20) + (5 paddings * 4)
#else
	int delta = 96; // (4 widgets * 20) + (4 paddings * 4)
#endif

	button     = new gButton(x(), y(), 20, 20, "", channelStop_xpm, channelPlay_xpm);
	mainButton = new gMidiChannelButton(button->x()+button->w()+4, y(), w() - delta, 20, "-- MIDI --");
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


void gMidiChannel::cb_button      (Fl_Widget *v, void *p) { ((gMidiChannel*)p)->__cb_button(); }
void gMidiChannel::cb_mute        (Fl_Widget *v, void *p) { ((gMidiChannel*)p)->__cb_mute(); }
void gMidiChannel::cb_solo        (Fl_Widget *v, void *p) { ((gMidiChannel*)p)->__cb_solo(); }
void gMidiChannel::cb_openMenu    (Fl_Widget *v, void *p) { ((gMidiChannel*)p)->__cb_openMenu(); }
void gMidiChannel::cb_changeVol   (Fl_Widget *v, void *p) { ((gMidiChannel*)p)->__cb_changeVol(); }
#ifdef WITH_VST
void gMidiChannel::cb_openFxWindow(Fl_Widget *v, void *p) { ((gMidiChannel*)p)->__cb_openFxWindow(); }
#endif


/* -------------------------------------------------------------------------- */


void gMidiChannel::__cb_mute()
{
	glue_setMute(ch);
}


/* -------------------------------------------------------------------------- */


void gMidiChannel::__cb_solo()
{
	solo->value() ? glue_setSoloOn(ch) : glue_setSoloOff(ch);
}


/* -------------------------------------------------------------------------- */


void gMidiChannel::__cb_changeVol()
{
	glue_setChanVol(ch, vol->value());
}


/* -------------------------------------------------------------------------- */


#ifdef WITH_VST
void gMidiChannel::__cb_openFxWindow()
{
	gu_openSubWindow(mainWin, new gdPluginList(PluginHost::CHANNEL, ch), WID_FX_LIST);
}
#endif

/* -------------------------------------------------------------------------- */


void gMidiChannel::__cb_button()
{
	if (button->value())
		glue_keyPress(ch, Fl::event_ctrl(), Fl::event_shift());
}


/* -------------------------------------------------------------------------- */


void gMidiChannel::__cb_openMenu()
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

	if (strcmp(m->label(), "Clone channel") == 0) {
		glue_cloneChannel(ch);
		return;
	}

	if (strcmp(m->label(), "Setup keyboard input...") == 0) {
		gu_openSubWindow(mainWin, new gdKeyGrabber(ch),	0);
		//new gdKeyGrabber(ch);
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

	if (strcmp(m->label(), "Setup MIDI input...") == 0) {
		gu_openSubWindow(mainWin, new gdMidiInputChannel(ch), 0);
		return;
	}

	if (strcmp(m->label(), "Setup MIDI output...") == 0) {
		//gu_openSubWindow(mainWin, new gdMidiGrabberChannel(ch, GrabForOutput), 0);
		gu_openSubWindow(mainWin, new gdMidiOutputMidiCh(ch), 0);
		return;
	}
}


/* -------------------------------------------------------------------------- */


void gMidiChannel::refresh()
{
	setColorsByStatus(ch->status, ch->recStatus);
	mainButton->redraw();
}


/* -------------------------------------------------------------------------- */


void gMidiChannel::reset()
{
	mainButton->setDefaultMode("-- MIDI --");
	mainButton->redraw();
}


/* -------------------------------------------------------------------------- */


void gMidiChannel::update()
{
	if (ch->midiOut) {
		char tmp[32];
		sprintf(tmp, "-- MIDI (channel %d) --", ch->midiOutChan+1);
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
#endif
}


/* -------------------------------------------------------------------------- */


void gMidiChannel::resize(int X, int Y, int W, int H)
{
  gChannel::resize(X, Y, W, H);

	/* this stuff makes sense only with FX button available. Do nothing
	 * otherwise */

#ifdef WITH_VST
	if (w() < BREAK_FX) {
		fx->hide();

		mainButton->size(w() - (BREAK_DELTA - BREAK_UNIT), mainButton->h());
	}
	else {
		fx->show();
		mainButton->size(w() - BREAK_DELTA, mainButton->h());
	}
	mute->resize(mainButton->x()+mainButton->w()+4, y(), 20, 20);
	solo->resize(mute->x()+mute->w()+4, y(), 20, 20);

	gChannel::init_sizes();
#endif
}


/* -------------------------------------------------------------------------- */


int gMidiChannel::keyPress(int e)
{
	return handleKey(e, ch->key);
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


gMidiChannelButton::gMidiChannelButton(int x, int y, int w, int h, const char *l)
	: gChannelButton(x, y, w, h, l) {}


/* -------------------------------------------------------------------------- */


int gMidiChannelButton::handle(int e)
{
	// MIDI drag-n-drop does nothing so far.
	return gClick::handle(e);
}
