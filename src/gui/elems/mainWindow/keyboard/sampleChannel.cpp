/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "glue/channel.h"
#include "glue/events.h"
#include "glue/io.h"
#include "glue/layout.h"
#include "glue/storage.h"
#include "gui/dispatcher.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/imageButton.h"
#include "gui/elems/basics/menu.h"
#include "gui/elems/mainWindow/keyboard/channelStatus.h"
#include "gui/elems/mainWindow/keyboard/column.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/elems/mainWindow/keyboard/midiActivity.h"
#include "gui/elems/mainWindow/keyboard/sampleChannelButton.h"
#include "gui/elems/mainWindow/keyboard/sampleChannelMode.h"
#include "gui/graphics.h"
#include "gui/ui.h"
#include "utils/gui.h"

extern giada::v::Ui g_ui;

namespace giada::v
{
namespace
{
enum class Menu
{
	INPUT_MONITOR = 0,
	OVERDUB_PROTECTION,
	LOAD_SAMPLE,
	EXPORT_SAMPLE,
	SETUP_KEYBOARD_INPUT,
	SETUP_MIDI_INPUT,
	SETUP_MIDI_OUTPUT,
	EDIT_ROUTING,
	EDIT_SAMPLE,
	EDIT_ACTIONS,
	CLEAR_ACTIONS,
	RENAME_CHANNEL,
	CLONE_CHANNEL,
	FREE_CHANNEL,
	DELETE_CHANNEL
};
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geSampleChannel::geSampleChannel(int X, int Y, int W, int H, c::channel::Data d)
: geChannel(X, Y, W, H, d)
{
	playButton     = new geImageButton(graphics::channelPlayOff, graphics::channelPlayOn);
	arm            = new geImageButton(graphics::armOff, graphics::armOn, graphics::armDisabled);
	status         = new geChannelStatus(0, 0, 0, 0, m_channel);
	mainButton     = new geSampleChannelButton(0, 0, 0, 0, m_channel);
	midiActivity   = new geMidiActivity();
	readActionsBtn = new geImageButton(graphics::readActionOff, graphics::readActionOn, graphics::readActionDisabled);
	modeBox        = new geSampleChannelMode(0, 0, 0, 0, m_channel);
	mute           = new geImageButton(graphics::muteOff, graphics::muteOn);
	solo           = new geImageButton(graphics::soloOff, graphics::soloOn);
	fx             = new geImageButton(graphics::fxOff, graphics::fxOn);
	vol            = new geDial(0, 0, 0, 0);

	add(playButton, G_GUI_UNIT);
	add(arm, G_GUI_UNIT);
	add(status, G_GUI_UNIT);
	add(mainButton);
	add(midiActivity, 10);
	add(readActionsBtn, G_GUI_UNIT);
	add(modeBox, G_GUI_UNIT);
	add(mute, G_GUI_UNIT);
	add(solo, G_GUI_UNIT);
	add(fx, G_GUI_UNIT);
	add(vol, G_GUI_UNIT);
	end();

	playButton->copy_tooltip(g_ui.getI18Text(LangMap::MAIN_CHANNEL_LABEL_PLAY));
	arm->copy_tooltip(g_ui.getI18Text(LangMap::MAIN_CHANNEL_LABEL_ARM));
	status->copy_tooltip(g_ui.getI18Text(LangMap::MAIN_CHANNEL_LABEL_STATUS));
	readActionsBtn->copy_tooltip(g_ui.getI18Text(LangMap::MAIN_CHANNEL_LABEL_READACTIONS));
	modeBox->copy_tooltip(g_ui.getI18Text(LangMap::MAIN_CHANNEL_LABEL_MODEBOX));
	mute->copy_tooltip(g_ui.getI18Text(LangMap::MAIN_CHANNEL_LABEL_MUTE));
	solo->copy_tooltip(g_ui.getI18Text(LangMap::MAIN_CHANNEL_LABEL_SOLO));
	fx->copy_tooltip(g_ui.getI18Text(LangMap::MAIN_CHANNEL_LABEL_FX));
	vol->copy_tooltip(g_ui.getI18Text(LangMap::MAIN_CHANNEL_LABEL_VOLUME));

	fx->forceValue(m_channel.plugins.size() > 0);
	fx->onClick = [this]() {
		c::layout::openChannelPluginListWindow(m_channel.id);
	};

	playButton->when(FL_WHEN_CHANGED); // On keypress && on keyrelease
	playButton->onClick = [this]() {
		g_ui.dispatcher.dispatchTouch(*this, playButton->getValue());
	};

	arm->setToggleable(true);
	arm->onClick = [this]() {
		c::events::toggleArmChannel(m_channel.id, Thread::MAIN);
	};

	mute->setToggleable(true);
	mute->onClick = [this]() {
		c::events::toggleMuteChannel(m_channel.id, Thread::MAIN);
	};

	solo->setToggleable(true);
	solo->onClick = [this]() {
		c::events::toggleSoloChannel(m_channel.id, Thread::MAIN);
	};

	mainButton->onClick = [this]() { openMenu(); };

	readActionsBtn->setToggleable(true);
	readActionsBtn->onClick = [this]() { readActions(); };

	vol->value(m_channel.volume);
	vol->callback(cb_changeVol, (void*)this);

	size(w(), h()); // Force responsiveness
}

/* -------------------------------------------------------------------------- */

void geSampleChannel::openMenu()
{
	/* If you're recording (input or actions) no menu is allowed; you can't do
	anything, especially deallocate the channel. */

	if (m_channel.isRecordingActions() || m_channel.isRecordingInput())
		return;

	geMenu menu;

	menu.addItem((ID)Menu::INPUT_MONITOR, g_ui.getI18Text(LangMap::MAIN_CHANNEL_MENU_INPUTMONITOR),
	    FL_MENU_TOGGLE | (m_channel.sample->inputMonitor ? FL_MENU_VALUE : 0));
	menu.addItem((ID)Menu::OVERDUB_PROTECTION, g_ui.getI18Text(LangMap::MAIN_CHANNEL_MENU_OVERDUBPROTECTION),
	    FL_MENU_TOGGLE | FL_MENU_DIVIDER | (m_channel.sample->overdubProtection ? FL_MENU_VALUE : 0));
	menu.addItem((ID)Menu::LOAD_SAMPLE, g_ui.getI18Text(LangMap::MAIN_CHANNEL_MENU_LOADSAMPLE));
	menu.addItem((ID)Menu::EXPORT_SAMPLE, g_ui.getI18Text(LangMap::MAIN_CHANNEL_MENU_EXPORTSAMPLE));
	menu.addItem((ID)Menu::SETUP_KEYBOARD_INPUT, g_ui.getI18Text(LangMap::MAIN_CHANNEL_MENU_KEYBOARDINPUT));
	menu.addItem((ID)Menu::SETUP_MIDI_INPUT, g_ui.getI18Text(LangMap::MAIN_CHANNEL_MENU_MIDIINPUT));
	menu.addItem((ID)Menu::SETUP_MIDI_OUTPUT, g_ui.getI18Text(LangMap::MAIN_CHANNEL_MENU_MIDIOUTPUT));
	menu.addItem((ID)Menu::EDIT_ROUTING, g_ui.getI18Text(LangMap::MAIN_CHANNEL_MENU_EDITROUTING));
	menu.addItem((ID)Menu::EDIT_SAMPLE, g_ui.getI18Text(LangMap::MAIN_CHANNEL_MENU_EDITSAMPLE));
	menu.addItem((ID)Menu::EDIT_ACTIONS, g_ui.getI18Text(LangMap::MAIN_CHANNEL_MENU_EDITACTIONS));
	menu.addItem((ID)Menu::CLEAR_ACTIONS, g_ui.getI18Text(LangMap::MAIN_CHANNEL_MENU_CLEARACTIONS));
	menu.addItem((ID)Menu::RENAME_CHANNEL, g_ui.getI18Text(LangMap::MAIN_CHANNEL_MENU_RENAME));
	menu.addItem((ID)Menu::CLONE_CHANNEL, g_ui.getI18Text(LangMap::MAIN_CHANNEL_MENU_CLONE));
	menu.addItem((ID)Menu::FREE_CHANNEL, g_ui.getI18Text(LangMap::MAIN_CHANNEL_MENU_FREE));
	menu.addItem((ID)Menu::DELETE_CHANNEL, g_ui.getI18Text(LangMap::MAIN_CHANNEL_MENU_DELETE));

	if (m_channel.sample->waveId == 0)
	{
		menu.setEnabled((ID)Menu::EXPORT_SAMPLE, false);
		menu.setEnabled((ID)Menu::EDIT_SAMPLE, false);
		menu.setEnabled((ID)Menu::FREE_CHANNEL, false);
		menu.setEnabled((ID)Menu::RENAME_CHANNEL, false);
	}

	if (!m_channel.hasActions)
		menu.setEnabled((ID)Menu::CLEAR_ACTIONS, false);

	menu.onSelect = [&channel = m_channel](ID id) {
		switch (static_cast<Menu>(id))
		{
		case Menu::INPUT_MONITOR:
			c::channel::setInputMonitor(channel.id, !channel.sample->inputMonitor);
			break;

		case Menu::OVERDUB_PROTECTION:
			c::channel::setOverdubProtection(channel.id, !channel.sample->overdubProtection);
			break;

		case Menu::LOAD_SAMPLE:
			c::layout::openBrowserForSampleLoad(channel.id);
			break;

		case Menu::EXPORT_SAMPLE:
			c::layout::openBrowserForSampleSave(channel.id);
			break;

		case Menu::SETUP_KEYBOARD_INPUT:
			c::layout::openKeyGrabberWindow(channel.key, [channelId = channel.id](int key) {
				return c::io::channel_setKey(channelId, key);
			});
			break;

		case Menu::SETUP_MIDI_INPUT:
			c::layout::openChannelMidiInputWindow(channel.id);
			break;

		case Menu::SETUP_MIDI_OUTPUT:
			c::layout::openSampleChannelMidiOutputWindow(channel.id);
			break;

		case Menu::EDIT_ROUTING:
			c::layout::openChannelRoutingWindow(channel.id);
			break;

		case Menu::EDIT_SAMPLE:
			c::layout::openSampleEditor(channel.id);
			break;

		case Menu::EDIT_ACTIONS:
			c::layout::openSampleActionEditor(channel.id);
			break;

		case Menu::CLEAR_ACTIONS:
			c::channel::clearAllActions(channel.id);
			break;

		case Menu::CLONE_CHANNEL:
			c::channel::cloneChannel(channel.id);
			break;

		case Menu::RENAME_CHANNEL:
			c::layout::openRenameChannelWindow(channel);
			break;

		case Menu::FREE_CHANNEL:
			c::channel::freeChannel(channel.id);
			break;

		case Menu::DELETE_CHANNEL:
			c::channel::deleteChannel(channel.id);
			break;
		}
	};

	menu.popup();
}

/* -------------------------------------------------------------------------- */

void geSampleChannel::readActions()
{
	if (Fl::event_shift())
		c::events::killReadActionsChannel(m_channel.id, Thread::MAIN);
	else
		c::events::toggleReadActionsChannel(m_channel.id, Thread::MAIN);
}

/* -------------------------------------------------------------------------- */

void geSampleChannel::refresh()
{
	geChannel::refresh();

	if (m_channel.sample->waveId != 0)
	{
		status->redraw();
		if (m_channel.sample->overdubProtection)
			arm->deactivate();
		else
			arm->activate();
	}

	if (m_channel.hasActions)
	{
		readActionsBtn->activate();
		readActionsBtn->setValue(m_channel.getReadActions());
	}
	else
		readActionsBtn->deactivate();
}

/* -------------------------------------------------------------------------- */

void geSampleChannel::draw()
{
	const int ny = y() + (h() / 2) - (G_GUI_UNIT / 2);

	modeBox->resize(modeBox->x(), ny, G_GUI_UNIT, G_GUI_UNIT);
	readActionsBtn->resize(readActionsBtn->x(), ny, G_GUI_UNIT, G_GUI_UNIT);

	geChannel::draw();
}

/* -------------------------------------------------------------------------- */

void geSampleChannel::resize(int X, int Y, int W, int H)
{
	geChannel::resize(X, Y, W, H);

	arm->hide();
	modeBox->hide();
	readActionsBtn->hide();
	fx->hide();

	if (w() > BREAK_ARM)
		arm->show();
	if (w() > BREAK_FX)
		fx->show();
	if (w() > BREAK_MODE_BOX)
		modeBox->show();
	if (w() > BREAK_READ_ACTIONS)
		readActionsBtn->show();
}
} // namespace giada::v