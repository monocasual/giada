/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#include <FL/Fl_Menu_Button.H>
#include "../../../../core/const.h"
#include "../../../../core/graphics.h"
#include "../../../../core/midiChannel.h"
#include "../../../../utils/gui.h"
#include "../../../../glue/channel.h"
#include "../../../../glue/io.h"
#include "../../../../glue/recorder.h"
#include "../../../dialogs/gd_mainWindow.h"
#include "../../../dialogs/sampleEditor.h"
#include "../../../dialogs/gd_actionEditor.h"
#include "../../../dialogs/gd_warnings.h"
#include "../../../dialogs/gd_keyGrabber.h"
#include "../../../dialogs/gd_pluginList.h"
#include "../../../dialogs/midiIO/midiInputChannel.h"
#include "../../../dialogs/midiIO/midiOutputMidiCh.h"
#include "../../basics/boxtypes.h"
#include "../../basics/idButton.h"
#include "../../basics/statusButton.h"
#include "../../basics/dial.h"
#include "midiChannel.h"


extern gdMainWindow *G_MainWin;


using namespace giada::m;


namespace
{
enum class Menu
{
  EDIT_ACTIONS = 0,
  CLEAR_ACTIONS,
  CLEAR_ACTIONS_ALL,
  __END_SUBMENU__,
  SETUP_KEYBOARD_INPUT,
  SETUP_MIDI_INPUT,
  SETUP_MIDI_OUTPUT,
  CLONE_CHANNEL,
  DELETE_CHANNEL
};


/* -------------------------------------------------------------------------- */


void menuCallback(Fl_Widget *w, void *v)
{
  geMidiChannel *gch = static_cast<geMidiChannel*>(w);
  Menu selectedItem = (Menu) (intptr_t) v;

  switch (selectedItem)
  {
    case Menu::CLEAR_ACTIONS:
    case Menu::__END_SUBMENU__:
      break;
    case Menu::EDIT_ACTIONS:
      gu_openSubWindow(G_MainWin, new gdActionEditor(gch->ch), WID_ACTION_EDITOR);
      break;
    case Menu::CLEAR_ACTIONS_ALL:
      glue_clearAllActions(gch);
      break;
    case Menu::SETUP_KEYBOARD_INPUT:
      gu_openSubWindow(G_MainWin, new gdKeyGrabber(gch->ch), 0);
      break;
    case Menu::SETUP_MIDI_INPUT:
      gu_openSubWindow(G_MainWin, new gdMidiInputChannel(gch->ch), 0);
      break;
    case Menu::SETUP_MIDI_OUTPUT:
      gu_openSubWindow(G_MainWin,
        new gdMidiOutputMidiCh(static_cast<MidiChannel*>(gch->ch)), 0);
      break;
    case Menu::CLONE_CHANNEL:
      glue_cloneChannel(gch->ch);
      break;
    case Menu::DELETE_CHANNEL:
      glue_deleteChannel(gch->ch);
      break;
  }
}

}; // {namespace}


/* -------------------------------------------------------------------------- */


geMidiChannel::geMidiChannel(int X, int Y, int W, int H, MidiChannel *ch)
	: geChannel(X, Y, W, H, CHANNEL_MIDI, ch)
{
	begin();

#if defined(WITH_VST)
  int delta = 144; // (6 widgets * 20) + (6 paddings * 4)
#else
	int delta = 120; // (5 widgets * 20) + (5 paddings * 4)
#endif

	button     = new geIdButton(x(), y(), 20, 20, "", channelStop_xpm, channelPlay_xpm);
	arm        = new geButton(button->x()+button->w()+4, y(), 20, 20, "", armOff_xpm, armOn_xpm);
	mainButton = new geMidiChannelButton(arm->x()+arm->w()+4, y(), w() - delta, 20, "-- MIDI --");
	mute       = new geButton(mainButton->x()+mainButton->w()+4, y(), 20, 20, "", muteOff_xpm, muteOn_xpm);
	solo       = new geButton(mute->x()+mute->w()+4, y(), 20, 20, "", soloOff_xpm, soloOn_xpm);
#if defined(WITH_VST)
	fx         = new geStatusButton(solo->x()+solo->w()+4, y(), 20, 20, fxOff_xpm, fxOn_xpm);
	vol        = new geDial(fx->x()+fx->w()+4, y(), 20, 20);
#else
	vol        = new geDial(solo->x()+solo->w()+4, y(), 20, 20);
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
		{"Edit actions...", 0, menuCallback, (void*) Menu::EDIT_ACTIONS},
		{"Clear actions",   0, menuCallback, (void*) Menu::CLEAR_ACTIONS, FL_SUBMENU},
			{"All",           0, menuCallback, (void*) Menu::CLEAR_ACTIONS_ALL},
			{0},
		{"Setup keyboard input...", 0, menuCallback, (void*) Menu::SETUP_KEYBOARD_INPUT},
		{"Setup MIDI input...",     0, menuCallback, (void*) Menu::SETUP_MIDI_INPUT},
		{"Setup MIDI output...",    0, menuCallback, (void*) Menu::SETUP_MIDI_OUTPUT},
		{"Clone channel",           0, menuCallback, (void*) Menu::CLONE_CHANNEL},
		{"Delete channel",          0, menuCallback, (void*) Menu::DELETE_CHANNEL},
		{0}
	};

	/* No 'clear actions' if there are no actions. */

	if (!ch->hasActions)
		rclick_menu[(int)Menu::CLEAR_ACTIONS].deactivate();

	Fl_Menu_Button *b = new Fl_Menu_Button(0, 0, 100, 50);
	b->box(G_CUSTOM_BORDER_BOX);
	b->textsize(GUI_FONT_SIZE_BASE);
	b->textcolor(COLOR_TEXT_0);
	b->color(G_COLOR_GREY_2);

  const Fl_Menu_Item *m = rclick_menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, b);
  if (m)
    m->do_callback(this, m->user_data());
  return;
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
	fx->status = ch->plugins.size() > 0;
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
	return geButton::handle(e);
}
