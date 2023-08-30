/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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
#include "glue/channel.h"
#include "glue/io.h"
#include "glue/layout.h"
#include "glue/main.h"
#include "gui/dialogs/warnings.h"
#include "gui/dispatcher.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/imageButton.h"
#include "gui/elems/basics/menu.h"
#include "gui/elems/mainWindow/keyboard/column.h"
#include "gui/elems/mainWindow/keyboard/midiChannelButton.h"
#include "gui/elems/midiActivity.h"
#include "gui/graphics.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include "utils/string.h"
#include <FL/Fl_Menu_Button.H>
#include <cassert>

extern giada::v::Ui* g_ui;

namespace giada::v
{
namespace
{
enum class Menu
{
	EDIT_ACTIONS = 0,
	CLEAR_ACTIONS,
	SETUP_KEYBOARD_INPUT,
	SETUP_MIDI_INPUT,
	SETUP_MIDI_OUTPUT,
	EDIT_ROUTING,
	RENAME_CHANNEL,
	CLONE_CHANNEL,
	DELETE_CHANNEL
};
} // namespace

/* -------------------------------------------------------------------------- */

geMidiChannel::geMidiChannel(int X, int Y, int W, int H, c::channel::Data d)
: geChannel(X, Y, W, H, d)
, m_data(d)
{
	playButton   = new geImageButton(graphics::channelPlayOff, graphics::channelPlayOn);
	arm          = new geImageButton(graphics::armOff, graphics::armOn);
	mainButton   = new geMidiChannelButton(0, 0, 0, 0, m_channel);
	midiActivity = new geMidiActivity();
	mute         = new geImageButton(graphics::muteOff, graphics::muteOn);
	solo         = new geImageButton(graphics::soloOff, graphics::soloOn);
	fx           = new geImageButton(graphics::fxOff, graphics::fxOn);
	vol          = new geDial(0, 0, 0, 0);

	add(playButton, G_GUI_UNIT);
	add(arm, G_GUI_UNIT);
	add(mainButton);
	add(midiActivity, 10);
	add(mute, G_GUI_UNIT);
	add(solo, G_GUI_UNIT);
	add(fx, G_GUI_UNIT);
	add(vol, G_GUI_UNIT);
	end();

	playButton->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_CHANNEL_LABEL_PLAY));
	arm->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_CHANNEL_LABEL_ARM));
	midiActivity->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_CHANNEL_LABEL_MIDIACTIVITY));
	mute->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_CHANNEL_LABEL_MUTE));
	solo->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_CHANNEL_LABEL_SOLO));
	fx->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_CHANNEL_LABEL_FX));
	vol->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_CHANNEL_LABEL_VOLUME));

	fx->setValue(m_channel.plugins.size() > 0);
	fx->onClick = [this]() {
		c::layout::openChannelPluginListWindow(m_channel.id);
	};

	playButton->when(FL_WHEN_CHANGED); // On keypress && on keyrelease
	playButton->onClick = [this]() {
		g_ui->dispatcher.dispatchTouch(*this, playButton->getValue());
	};

	arm->setToggleable(true);
	arm->onClick = [this]() {
		c::channel::toggleArmChannel(m_channel.id, Thread::MAIN);
	};

	mute->setToggleable(true);
	mute->onClick = [this]() {
		c::channel::toggleMuteChannel(m_channel.id, Thread::MAIN);
	};

	solo->setToggleable(true);
	solo->onClick = [this]() {
		c::channel::toggleSoloChannel(m_channel.id, Thread::MAIN);
	};

	mainButton->onClick = [this]() { openMenu(); };

	vol->value(m_channel.volume);
	vol->callback(cb_changeVol, (void*)this);

	size(w(), h()); // Force responsiveness
}

/* -------------------------------------------------------------------------- */

void geMidiChannel::openMenu()
{
	geMenu menu;

	menu.addItem((ID)Menu::EDIT_ACTIONS, g_ui->getI18Text(LangMap::MAIN_CHANNEL_MENU_EDITACTIONS));
	menu.addItem((ID)Menu::CLEAR_ACTIONS, g_ui->getI18Text(LangMap::MAIN_CHANNEL_MENU_CLEARACTIONS));
	menu.addItem((ID)Menu::SETUP_KEYBOARD_INPUT, g_ui->getI18Text(LangMap::MAIN_CHANNEL_MENU_KEYBOARDINPUT));
	menu.addItem((ID)Menu::SETUP_MIDI_INPUT, g_ui->getI18Text(LangMap::MAIN_CHANNEL_MENU_MIDIINPUT));
	menu.addItem((ID)Menu::SETUP_MIDI_OUTPUT, g_ui->getI18Text(LangMap::MAIN_CHANNEL_MENU_MIDIOUTPUT));
	menu.addItem((ID)Menu::EDIT_ROUTING, g_ui->getI18Text(LangMap::MAIN_CHANNEL_MENU_EDITROUTING));
	menu.addItem((ID)Menu::RENAME_CHANNEL, g_ui->getI18Text(LangMap::MAIN_CHANNEL_MENU_RENAME));
	menu.addItem((ID)Menu::CLONE_CHANNEL, g_ui->getI18Text(LangMap::MAIN_CHANNEL_MENU_CLONE));
	menu.addItem((ID)Menu::DELETE_CHANNEL, g_ui->getI18Text(LangMap::MAIN_CHANNEL_MENU_DELETE));

	if (!m_data.hasActions)
		menu.setEnabled((ID)Menu::CLEAR_ACTIONS, false);

	menu.onSelect = [&data = m_data](ID id) {
		switch (static_cast<Menu>(id))
		{
		case Menu::EDIT_ACTIONS:
			c::layout::openMidiActionEditor(data.id);
			break;
		case Menu::CLEAR_ACTIONS:
			c::channel::clearAllActions(data.id);
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
		case Menu::EDIT_ROUTING:
			c::layout::openChannelRoutingWindow(data.id);
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
	};

	menu.popup();
}

/* -------------------------------------------------------------------------- */

void geMidiChannel::resize(int X, int Y, int W, int H)
{
	geChannel::resize(X, Y, W, H);

	arm->hide();
	fx->hide();

	if (w() > BREAK_MINI)
		arm->show();
	if (w() > BREAK_SMALL)
		fx->show();
}
} // namespace giada::v