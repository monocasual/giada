/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) G_GUI_UNIT10-G_GUI_UNIT17 Giovanni A. Zuliani | Monocasual
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


#include <cassert>
#include <FL/Fl_Menu_Button.H>
#include "core/const.h"
#include "core/graphics.h"
#include "core/channels/midiChannel.h"
#include "core/model/model.h"
#include "core/recorder.h"
#include "utils/gui.h"
#include "utils/string.h"
#include "glue/channel.h"
#include "glue/io.h"
#include "glue/recorder.h"
#include "gui/dispatcher.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/channelNameInput.h"
#include "gui/dialogs/warnings.h"
#include "gui/dialogs/keyGrabber.h"
#include "gui/dialogs/pluginList.h"
#include "gui/dialogs/actionEditor/midiActionEditor.h"
#include "gui/dialogs/midiIO/midiInputChannel.h"
#include "gui/dialogs/midiIO/midiOutputMidiCh.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/statusButton.h"
#include "gui/elems/basics/dial.h"
#include "column.h"
#include "midiChannelButton.h"
#include "midiChannel.h"


extern giada::v::gdMainWindow* G_MainWin;


namespace giada {
namespace v
{
namespace
{
enum class Menu
{
	EDIT_ACTIONS = 0,
	CLEAR_ACTIONS,
	CLEAR_ACTIONS_ALL,
	__END_CLEAR_ACTION_SUBMENU__,
	SETUP_KEYBOARD_INPUT,
	SETUP_MIDI_INPUT,
	SETUP_MIDI_OUTPUT,
	RENAME_CHANNEL,
	CLONE_CHANNEL,
	DELETE_CHANNEL
};


/* -------------------------------------------------------------------------- */


void menuCallback(Fl_Widget* w, void* v)
{
	geMidiChannel* gch = static_cast<geMidiChannel*>(w);

	Menu selectedItem = (Menu) (intptr_t) v;

	switch (selectedItem)
	{
		case Menu::CLEAR_ACTIONS:
		case Menu::__END_CLEAR_ACTION_SUBMENU__:
			break;
		case Menu::EDIT_ACTIONS:
			u::gui::openSubWindow(G_MainWin, new v::gdMidiActionEditor(gch->channelId), WID_ACTION_EDITOR);
			break;
		case Menu::CLEAR_ACTIONS_ALL:
			c::recorder::clearAllActions(gch->channelId);
			break;
		case Menu::SETUP_KEYBOARD_INPUT:
			u::gui::openSubWindow(G_MainWin, new gdKeyGrabber(gch->channelId), WID_KEY_GRABBER);
			break;
		case Menu::SETUP_MIDI_INPUT:
			u::gui::openSubWindow(G_MainWin, new gdMidiInputChannel(gch->channelId), WID_MIDI_INPUT);
			break;
		case Menu::SETUP_MIDI_OUTPUT:
			u::gui::openSubWindow(G_MainWin, new gdMidiOutputMidiCh(gch->channelId), WID_MIDI_OUTPUT);
			break;
		case Menu::CLONE_CHANNEL:
			c::channel::cloneChannel(gch->channelId);
			break;		
		case Menu::RENAME_CHANNEL:
			u::gui::openSubWindow(G_MainWin, new gdChannelNameInput(gch->channelId), WID_SAMPLE_NAME);
			break;
		case Menu::DELETE_CHANNEL:
			c::channel::deleteChannel(gch->channelId);
			break;
	}
}
} // {anonymous}


/* -------------------------------------------------------------------------- */


geMidiChannel::geMidiChannel(int X, int Y, int W, int H, ID channelId)
: geChannel(X, Y, W, H, channelId)
{
#if defined(WITH_VST)
	constexpr int delta = 6 * (G_GUI_UNIT + G_GUI_INNER_MARGIN);
#else
	constexpr int delta = 5 * (G_GUI_UNIT + G_GUI_INNER_MARGIN);
#endif

	playButton = new geStatusButton     (x(), y(), G_GUI_UNIT, G_GUI_UNIT, channelStop_xpm, channelPlay_xpm);
	arm        = new geButton           (playButton->x() + playButton->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, G_GUI_UNIT, "", armOff_xpm, armOn_xpm);
	mainButton = new geMidiChannelButton(arm->x() + arm->w() + G_GUI_INNER_MARGIN, y(), w() - delta, H, channelId);
	mute       = new geStatusButton     (mainButton->x() + mainButton->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, G_GUI_UNIT, muteOff_xpm, muteOn_xpm);
	solo       = new geStatusButton     (mute->x() + mute->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, G_GUI_UNIT, soloOff_xpm, soloOn_xpm);
#if defined(WITH_VST)
	fx         = new geStatusButton     (solo->x() + solo->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, G_GUI_UNIT, fxOff_xpm, fxOn_xpm);
	vol        = new geDial             (fx->x() + fx->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, G_GUI_UNIT);
#else
	vol        = new geDial             (solo->x() + solo->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, G_GUI_UNIT);
#endif

	end();


	resizable(mainButton);

	m::model::ChannelsLock l(m::model::channels);
	const m::Channel& ch = m::model::get(m::model::channels, channelId);

#ifdef WITH_VST
	fx->setStatus(ch.pluginIds.size() > 0);
	fx->tooltip("FX");
#endif
	playButton->tooltip("Trigger MIDI");
	arm->tooltip("Arm input recording");
	mainButton->tooltip("Channel menu");
	mute->tooltip("Toggle mute");
	solo->tooltip("Toggle solo");

	playButton->callback(cb_playButton, (void*)this);
	playButton->when(FL_WHEN_CHANGED);   // On keypress && on keyrelease

	arm->type(FL_TOGGLE_BUTTON);
	arm->value(ch.armed);
	arm->callback(cb_arm, (void*)this);

#ifdef WITH_VST
	fx->callback(cb_openFxWindow, (void*)this);
#endif

	mute->type(FL_TOGGLE_BUTTON);
	mute->callback(cb_mute, (void*)this);

	solo->type(FL_TOGGLE_BUTTON);
	solo->callback(cb_solo, (void*)this);

	mainButton->setKey(ch.key);
	mainButton->callback(cb_openMenu, (void*)this);

	vol->value(ch.volume);
	vol->callback(cb_changeVol, (void*)this);

	size(w(), h()); // Force responsiveness
}


/* -------------------------------------------------------------------------- */


void geMidiChannel::cb_playButton(Fl_Widget* v, void* p) { ((geMidiChannel*)p)->cb_playButton(); }
void geMidiChannel::cb_openMenu(Fl_Widget* v, void* p) { ((geMidiChannel*)p)->cb_openMenu(); }


/* -------------------------------------------------------------------------- */


void geMidiChannel::cb_playButton()
{
	v::dispatcher::dispatchTouch(this, playButton->value());
}


/* -------------------------------------------------------------------------- */


void geMidiChannel::cb_openMenu()
{
	Fl_Menu_Item rclick_menu[] = {
		{"Edit actions...", 0, menuCallback, (void*) Menu::EDIT_ACTIONS},
		{"Clear actions",   0, menuCallback, (void*) Menu::CLEAR_ACTIONS, FL_SUBMENU},
			{"All",           0, menuCallback, (void*) Menu::CLEAR_ACTIONS_ALL},
			{0},
		{"Setup keyboard input...", 0, menuCallback, (void*) Menu::SETUP_KEYBOARD_INPUT},
		{"Setup MIDI input...",     0, menuCallback, (void*) Menu::SETUP_MIDI_INPUT},
		{"Setup MIDI output...",    0, menuCallback, (void*) Menu::SETUP_MIDI_OUTPUT},
		{"Rename", 0, menuCallback, (void*) Menu::RENAME_CHANNEL},
		{"Clone",  0, menuCallback, (void*) Menu::CLONE_CHANNEL},
		{"Delete", 0, menuCallback, (void*) Menu::DELETE_CHANNEL},
		{0}
	};

	/* No 'clear actions' if there are no actions. */

	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c)
	{
		if (!c.hasActions)
			rclick_menu[(int)Menu::CLEAR_ACTIONS].deactivate();
	});

	Fl_Menu_Button b(0, 0, 100, 50);
	b.box(G_CUSTOM_BORDER_BOX);
	b.textsize(G_GUI_FONT_SIZE_BASE);
	b.textcolor(G_COLOR_LIGHT_2);
	b.color(G_COLOR_GREY_2);

	const Fl_Menu_Item* m = rclick_menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, &b);
	if (m != nullptr)
		m->do_callback(this, m->user_data());
	return;
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

}} // giada::v::
