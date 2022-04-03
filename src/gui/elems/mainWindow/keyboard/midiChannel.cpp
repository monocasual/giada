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

#include "gui/elems/mainWindow/keyboard/midiChannel.h"
#include "core/const.h"
#include "core/graphics.h"
#include "glue/channel.h"
#include "glue/io.h"
#include "glue/layout.h"
#include "glue/recorder.h"
#include "gui/dialogs/warnings.h"
#include "gui/dispatcher.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/statusButton.h"
#include "gui/elems/mainWindow/keyboard/column.h"
#include "gui/elems/mainWindow/keyboard/midiActivity.h"
#include "gui/elems/mainWindow/keyboard/midiChannelButton.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include "utils/string.h"
#include <FL/Fl_Menu_Button.H>
#include <cassert>

extern giada::v::Ui g_ui;

namespace giada::v
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
	const geMidiChannel*    gch  = static_cast<geMidiChannel*>(w);
	const c::channel::Data& data = gch->getData();

	switch ((Menu)(intptr_t)v)
	{
	case Menu::CLEAR_ACTIONS:
	case Menu::__END_CLEAR_ACTION_SUBMENU__:
		break;
	case Menu::EDIT_ACTIONS:
		c::layout::openMidiActionEditor(data.id);
		break;
	case Menu::CLEAR_ACTIONS_ALL:
		c::recorder::clearAllActions(data.id);
		break;
	case Menu::SETUP_KEYBOARD_INPUT:
		c::layout::openKeyGrabberWindow(data.key, [channelId = data.id](int key) {
			return c::io::channel_setKey(channelId, key);
		});
		break;
	case Menu::SETUP_MIDI_INPUT:
		c::layout::openChannelMidiInputWindow(data.id);
		break;
	case Menu::SETUP_MIDI_OUTPUT:
		c::layout::openMidiChannelMidiOutputWindow(data.id);
		break;
	case Menu::CLONE_CHANNEL:
		c::channel::cloneChannel(data.id);
		break;
	case Menu::RENAME_CHANNEL:
		c::layout::openRenameChannelWindow(data);
		break;
	case Menu::DELETE_CHANNEL:
		c::channel::deleteChannel(data.id);
		break;
	}
}
} // namespace

/* -------------------------------------------------------------------------- */

geMidiChannel::geMidiChannel(int X, int Y, int W, int H, c::channel::Data d)
: geChannel(X, Y, W, H, d)
, m_data(d)
{
	playButton   = new geStatusButton(x(), y(), G_GUI_UNIT, G_GUI_UNIT, channelStop_xpm, channelPlay_xpm);
	arm          = new geButton(playButton->x() + playButton->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, G_GUI_UNIT, "", armOff_xpm, armOn_xpm);
	mainButton   = new geMidiChannelButton(arm->x() + arm->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, H, m_channel);
	midiActivity = new geMidiActivity(mainButton->x() + mainButton->w() + G_GUI_INNER_MARGIN, y(), 10, h());
	mute         = new geStatusButton(midiActivity->x() + midiActivity->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, G_GUI_UNIT, muteOff_xpm, muteOn_xpm);
	solo         = new geStatusButton(mute->x() + mute->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, G_GUI_UNIT, soloOff_xpm, soloOn_xpm);
#if defined(WITH_VST)
	fx  = new geStatusButton(solo->x() + solo->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, G_GUI_UNIT, fxOff_xpm, fxOn_xpm);
	vol = new geDial(fx->x() + fx->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, G_GUI_UNIT);
#else
	vol = new geDial(solo->x() + solo->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, G_GUI_UNIT);
#endif

	end();

	resizable(mainButton);

	playButton->copy_tooltip(g_ui.langMapper.get(LangMap::MAIN_CHANNEL_LABEL_PLAY));
	arm->copy_tooltip(g_ui.langMapper.get(LangMap::MAIN_CHANNEL_LABEL_ARM));
	mute->copy_tooltip(g_ui.langMapper.get(LangMap::MAIN_CHANNEL_LABEL_MUTE));
	solo->copy_tooltip(g_ui.langMapper.get(LangMap::MAIN_CHANNEL_LABEL_SOLO));
#if defined(WITH_VST)
	fx->copy_tooltip(g_ui.langMapper.get(LangMap::MAIN_CHANNEL_LABEL_FX));
#endif
	vol->copy_tooltip(g_ui.langMapper.get(LangMap::MAIN_CHANNEL_LABEL_VOLUME));

#ifdef WITH_VST
	fx->setStatus(m_channel.plugins.size() > 0);
#endif

	playButton->callback(cb_playButton, (void*)this);
	playButton->when(FL_WHEN_CHANGED); // On keypress && on keyrelease

	arm->type(FL_TOGGLE_BUTTON);
	arm->value(m_channel.isArmed());
	arm->callback(cb_arm, (void*)this);

#ifdef WITH_VST
	fx->callback(cb_openFxWindow, (void*)this);
#endif

	mute->type(FL_TOGGLE_BUTTON);
	mute->callback(cb_mute, (void*)this);

	solo->type(FL_TOGGLE_BUTTON);
	solo->callback(cb_solo, (void*)this);

	mainButton->callback(cb_openMenu, (void*)this);

	vol->value(m_channel.volume);
	vol->callback(cb_changeVol, (void*)this);

	size(w(), h()); // Force responsiveness
}

/* -------------------------------------------------------------------------- */

void geMidiChannel::cb_playButton(Fl_Widget* /*w*/, void* p) { ((geMidiChannel*)p)->cb_playButton(); }
void geMidiChannel::cb_openMenu(Fl_Widget* /*w*/, void* p) { ((geMidiChannel*)p)->cb_openMenu(); }

/* -------------------------------------------------------------------------- */

void geMidiChannel::cb_playButton()
{
	m_channel.viewDispatcher.dispatchTouch(*this, playButton->value());
}

/* -------------------------------------------------------------------------- */

void geMidiChannel::cb_openMenu()
{
	Fl_Menu_Item rclick_menu[] = {
	    u::gui::makeMenuItem(g_ui.langMapper.get(LangMap::MAIN_CHANNEL_MENU_EDITACTIONS), menuCallback, (void*)Menu::EDIT_ACTIONS),
	    u::gui::makeMenuItem(g_ui.langMapper.get(LangMap::MAIN_CHANNEL_MENU_CLEARACTIONS), menuCallback, (void*)Menu::CLEAR_ACTIONS, FL_SUBMENU),
	    u::gui::makeMenuItem(g_ui.langMapper.get(LangMap::MAIN_CHANNEL_MENU_CLEARACTIONS_ALL), menuCallback, (void*)Menu::CLEAR_ACTIONS_ALL),
	    {},
	    u::gui::makeMenuItem(g_ui.langMapper.get(LangMap::MAIN_CHANNEL_MENU_KEYBOARDINPUT), menuCallback, (void*)Menu::SETUP_KEYBOARD_INPUT),
	    u::gui::makeMenuItem(g_ui.langMapper.get(LangMap::MAIN_CHANNEL_MENU_MIDIINPUT), menuCallback, (void*)Menu::SETUP_MIDI_INPUT),
	    u::gui::makeMenuItem(g_ui.langMapper.get(LangMap::MAIN_CHANNEL_MENU_MIDIOUTPUT), menuCallback, (void*)Menu::SETUP_MIDI_OUTPUT),
	    u::gui::makeMenuItem(g_ui.langMapper.get(LangMap::MAIN_CHANNEL_MENU_RENAME), menuCallback, (void*)Menu::RENAME_CHANNEL),
	    u::gui::makeMenuItem(g_ui.langMapper.get(LangMap::MAIN_CHANNEL_MENU_CLONE), menuCallback, (void*)Menu::CLONE_CHANNEL),
	    u::gui::makeMenuItem(g_ui.langMapper.get(LangMap::MAIN_CHANNEL_MENU_DELETE), menuCallback, (void*)Menu::DELETE_CHANNEL),
	    {}};

	/* No 'clear actions' if there are no actions. */

	if (!m_data.hasActions)
		rclick_menu[(int)Menu::CLEAR_ACTIONS].deactivate();

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
} // namespace giada::v