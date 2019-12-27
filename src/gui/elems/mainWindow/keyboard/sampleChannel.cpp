/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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
#include "core/channels/sampleChannel.h"
#include "core/model/model.h"
#include "core/mixer.h"
#include "core/conf.h"
#include "core/clock.h"
#include "core/graphics.h"
#include "core/wave.h"
#include "core/recorder.h"
#include "core/recManager.h"
#include "glue/io.h"
#include "glue/channel.h"
#include "glue/recorder.h"
#include "glue/storage.h"
#include "utils/gui.h"
#include "gui/dispatcher.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/keyGrabber.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/dialogs/channelNameInput.h"
#include "gui/dialogs/warnings.h"
#include "gui/dialogs/actionEditor/sampleActionEditor.h"
#include "gui/dialogs/browser/browserSave.h"
#include "gui/dialogs/browser/browserLoad.h"
#include "gui/dialogs/midiIO/midiOutputSampleCh.h"
#include "gui/dialogs/midiIO/midiInputChannel.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/statusButton.h"
#include "gui/elems/basics/dial.h"
#include "channelStatus.h"
#include "channelMode.h"
#include "sampleChannelButton.h"
#include "keyboard.h"
#include "column.h"
#include "sampleChannel.h"


extern giada::v::gdMainWindow* G_MainWin;


namespace giada {
namespace v
{
namespace
{
enum class Menu
{
	INPUT_MONITOR = 0,
	LOAD_SAMPLE,
	EXPORT_SAMPLE,
	SETUP_KEYBOARD_INPUT,
	SETUP_MIDI_INPUT,
	SETUP_MIDI_OUTPUT,
	EDIT_SAMPLE,
	EDIT_ACTIONS,
	CLEAR_ACTIONS,
	CLEAR_ACTIONS_ALL,
	CLEAR_ACTIONS_VOLUME,
	CLEAR_ACTIONS_START_STOP,
	__END_CLEAR_ACTIONS_SUBMENU__,
	/*RESIZE,
	RESIZE_H1,
	RESIZE_H2,
	RESIZE_H3,
	RESIZE_H4,
	__END_RESIZE_SUBMENU__,*/
	RENAME_CHANNEL,
	CLONE_CHANNEL,
	FREE_CHANNEL,
	DELETE_CHANNEL
};


/* -------------------------------------------------------------------------- */


void menuCallback(Fl_Widget* w, void* v)
{
	geSampleChannel* gch = static_cast<geSampleChannel*>(w);

	ID    waveId;
	bool inputMonitor;
	m::model::onGet(m::model::channels, gch->channelId, [&](m::Channel& c)
	{
		waveId       = static_cast<m::SampleChannel&>(c).waveId;
		inputMonitor = static_cast<m::SampleChannel&>(c).inputMonitor;
	});

	Menu selectedItem = (Menu) (intptr_t) v;

	switch (selectedItem) {
		case Menu::INPUT_MONITOR: {
			c::channel::setInputMonitor(gch->channelId, !inputMonitor);
			break;
		}
		case Menu::LOAD_SAMPLE: {
			gdWindow* w = new gdBrowserLoad("Browse sample", 
				m::conf::conf.samplePath.c_str(), c::storage::loadSample, gch->channelId);
			u::gui::openSubWindow(G_MainWin, w, WID_FILE_BROWSER);
			break;
		}
		case Menu::EXPORT_SAMPLE: {
			gdWindow* w = new gdBrowserSave("Save sample", 
				m::conf::conf.samplePath.c_str(), "", c::storage::saveSample, gch->channelId);
			u::gui::openSubWindow(G_MainWin, w, WID_FILE_BROWSER);
			break;
		}
		case Menu::SETUP_KEYBOARD_INPUT: {
			u::gui::openSubWindow(G_MainWin, new gdKeyGrabber(gch->channelId), 
				WID_KEY_GRABBER);
			break;
		}
		case Menu::SETUP_MIDI_INPUT: {
			u::gui::openSubWindow(G_MainWin, new gdMidiInputChannel(gch->channelId), 
				WID_MIDI_INPUT);
			break;
		}
		case Menu::SETUP_MIDI_OUTPUT: {
			u::gui::openSubWindow(G_MainWin, new gdMidiOutputSampleCh(gch->channelId), 
				WID_MIDI_OUTPUT);
			break;
		}
		case Menu::EDIT_SAMPLE: {
			u::gui::openSubWindow(G_MainWin, new gdSampleEditor(gch->channelId, waveId), 
				WID_SAMPLE_EDITOR);
			break;
		}
		case Menu::EDIT_ACTIONS: {
			u::gui::openSubWindow(G_MainWin, new gdSampleActionEditor(gch->channelId), 
				WID_ACTION_EDITOR);
			break;
		}
		case Menu::CLEAR_ACTIONS:
		case Menu::__END_CLEAR_ACTIONS_SUBMENU__:
		//case Menu::RESIZE:
		//case Menu::__END_RESIZE_SUBMENU__:
			break;
		case Menu::CLEAR_ACTIONS_ALL: {
			c::recorder::clearAllActions(gch->channelId);
			break;
		}
		case Menu::CLEAR_ACTIONS_VOLUME: {
			c::recorder::clearVolumeActions(gch->channelId);
			break;
		}
		case Menu::CLEAR_ACTIONS_START_STOP: {
			c::recorder::clearStartStopActions(gch->channelId);
			break;
		}
		/*case Menu::RESIZE_H1: {
			gch->changeSize(G_GUI_CHANNEL_H_1);
			break;
		}		
		case Menu::RESIZE_H2: {
			gch->changeSize(G_GUI_CHANNEL_H_2);
			break;
		}		
		case Menu::RESIZE_H3: {
			gch->changeSize(G_GUI_CHANNEL_H_3);
			break;
		}		
		case Menu::RESIZE_H4: {
			gch->changeSize(G_GUI_CHANNEL_H_4);
			break;
		}*/
		case Menu::CLONE_CHANNEL: {
			c::channel::cloneChannel(gch->channelId);
			break;
		}
		case Menu::RENAME_CHANNEL: {
			u::gui::openSubWindow(G_MainWin, new gdChannelNameInput(gch->channelId), 
				WID_SAMPLE_NAME);
			break;
		}
		case Menu::FREE_CHANNEL: {
			c::channel::freeChannel(gch->channelId);
			break;
		}
		case Menu::DELETE_CHANNEL: {
			c::channel::deleteChannel(gch->channelId);
			break;
		}
	}
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


geSampleChannel::geSampleChannel(int X, int Y, int W, int H, ID channelId)
	: geChannel(X, Y, W, H, channelId)
{
	playButton  = new geStatusButton(0, 0, G_GUI_UNIT, G_GUI_UNIT, channelStop_xpm, channelPlay_xpm);
	arm         = new geButton(0, 0, G_GUI_UNIT, G_GUI_UNIT, "", armOff_xpm, armOn_xpm);
	status      = new geChannelStatus(0, 0, G_GUI_UNIT, H, channelId);
	mainButton  = new geSampleChannelButton(0, 0, G_GUI_UNIT, H, channelId);
	readActions = new geStatusButton(0, 0, G_GUI_UNIT, G_GUI_UNIT, readActionOff_xpm, readActionOn_xpm);
	modeBox     = new geChannelMode(0, 0, G_GUI_UNIT, G_GUI_UNIT, channelId);
	mute        = new geStatusButton(0, 0, G_GUI_UNIT, G_GUI_UNIT, muteOff_xpm, muteOn_xpm);
	solo        = new geStatusButton(0, 0, G_GUI_UNIT, G_GUI_UNIT, soloOff_xpm, soloOn_xpm);
#ifdef WITH_VST
	fx          = new geStatusButton(0, 0, G_GUI_UNIT, G_GUI_UNIT, fxOff_xpm, fxOn_xpm);
#endif
	vol         = new geDial(0, 0, G_GUI_UNIT, G_GUI_UNIT);

	end();

	resizable(mainButton);

	m::model::ChannelsLock l(m::model::channels);
	const m::SampleChannel& ch = static_cast<m::SampleChannel&>(m::model::get(m::model::channels, channelId));

	modeBox->value(static_cast<int>(ch.mode));
	modeBox->redraw();

#ifdef WITH_VST
	fx->setStatus(ch.pluginIds.size() > 0);
#endif

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

	//readActions->type(FL_TOGGLE_BUTTON);
	//readActions->value(ch.readActions);
	readActions->setStatus(ch.readActions);
	readActions->callback(cb_readActions, (void*)this);

	vol->value(ch.volume);
	vol->callback(cb_changeVol, (void*)this);

	changeSize(H);  // Update size dynamically
}


/* -------------------------------------------------------------------------- */


void geSampleChannel::cb_playButton (Fl_Widget* v, void* p) { ((geSampleChannel*)p)->cb_playButton(); }
void geSampleChannel::cb_openMenu   (Fl_Widget* v, void* p) { ((geSampleChannel*)p)->cb_openMenu(); }
void geSampleChannel::cb_readActions(Fl_Widget* v, void* p) { ((geSampleChannel*)p)->cb_readActions(); }


/* -------------------------------------------------------------------------- */


void geSampleChannel::cb_playButton()
{
	v::dispatcher::dispatchTouch(this, playButton->value());
}


/* -------------------------------------------------------------------------- */


void geSampleChannel::cb_openMenu()
{
	bool inputMonitor;
	bool isEmptyOrMissing;
	bool hasActions;
	bool isAnyLoopMode;
	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c)
	{
		const m::SampleChannel& sc = static_cast<m::SampleChannel&>(c);
		inputMonitor     = sc.inputMonitor;
		isEmptyOrMissing = sc.playStatus == ChannelStatus::EMPTY || sc.playStatus == ChannelStatus::MISSING;
		hasActions       = sc.hasActions;
		isAnyLoopMode    = sc.isAnyLoopMode();
	});

	/* If you're recording (input or actions) no menu is allowed; you can't do
	anything, especially deallocate the channel */

	if (m::recManager::isRecording())
		return;

	Fl_Menu_Item rclick_menu[] = {
		{"Input monitor",            0, menuCallback, (void*) Menu::INPUT_MONITOR,
			FL_MENU_TOGGLE | FL_MENU_DIVIDER | (inputMonitor ? FL_MENU_VALUE : 0)},
		{"Load new sample...",       0, menuCallback, (void*) Menu::LOAD_SAMPLE},
		{"Export sample to file...", 0, menuCallback, (void*) Menu::EXPORT_SAMPLE},
		{"Setup keyboard input...",  0, menuCallback, (void*) Menu::SETUP_KEYBOARD_INPUT},
		{"Setup MIDI input...",      0, menuCallback, (void*) Menu::SETUP_MIDI_INPUT},
		{"Setup MIDI output...",     0, menuCallback, (void*) Menu::SETUP_MIDI_OUTPUT},
		{"Edit sample...",           0, menuCallback, (void*) Menu::EDIT_SAMPLE},
		{"Edit actions...",          0, menuCallback, (void*) Menu::EDIT_ACTIONS},
		{"Clear actions",            0, menuCallback, (void*) Menu::CLEAR_ACTIONS, FL_SUBMENU},
			{"All",        0, menuCallback, (void*) Menu::CLEAR_ACTIONS_ALL},
			{"Volume",     0, menuCallback, (void*) Menu::CLEAR_ACTIONS_VOLUME},
			{"Start/Stop", 0, menuCallback, (void*) Menu::CLEAR_ACTIONS_START_STOP},
			{0},
/*		{"Resize",    0, menuCallback, (void*) Menu::RESIZE, FL_SUBMENU},
			{"Normal",  0, menuCallback, (void*) Menu::RESIZE_H1},
			{"Medium",  0, menuCallback, (void*) Menu::RESIZE_H2},
			{"Large",   0, menuCallback, (void*) Menu::RESIZE_H3},
			{"X-Large", 0, menuCallback, (void*) Menu::RESIZE_H4},
			{0},*/
		{"Rename", 0, menuCallback, (void*) Menu::RENAME_CHANNEL},
		{"Clone",  0, menuCallback, (void*) Menu::CLONE_CHANNEL},
		{"Free",   0, menuCallback, (void*) Menu::FREE_CHANNEL},
		{"Delete", 0, menuCallback, (void*) Menu::DELETE_CHANNEL},
		{0}
	};

	if (isEmptyOrMissing) {
		rclick_menu[(int) Menu::EXPORT_SAMPLE].deactivate();
		rclick_menu[(int) Menu::EDIT_SAMPLE].deactivate();
		rclick_menu[(int) Menu::FREE_CHANNEL].deactivate();
		rclick_menu[(int) Menu::RENAME_CHANNEL].deactivate();
	}

	if (!hasActions)
		rclick_menu[(int) Menu::CLEAR_ACTIONS].deactivate();


	/* No 'clear start/stop actions' for those channels in loop mode: they cannot
	have start/stop actions. */

	if (isAnyLoopMode)
		rclick_menu[(int) Menu::CLEAR_ACTIONS_START_STOP].deactivate();

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


void geSampleChannel::cb_readActions()
{
	c::channel::toggleReadingActions(channelId);
}


/* -------------------------------------------------------------------------- */


void geSampleChannel::refresh()
{
	geChannel::refresh();

	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c)
	{
		if (c.hasData()) 
			status->redraw();

		if (c.hasActions) {
			readActions->show();
			readActions->setStatus(c.readActions);
			readActions->redraw();
		}
		else 
			readActions->hide();
	});
}


/* -------------------------------------------------------------------------- */


void geSampleChannel::resize(int X, int Y, int W, int H)
{
	geChannel::resize(X, Y, W, H);

	arm->hide();
	modeBox->hide();
	readActions->hide();
#ifdef WITH_VST
	fx->hide();
#endif
	
	m::model::ChannelsLock l(m::model::channels);
	const m::SampleChannel& ch = static_cast<m::SampleChannel&>(m::model::get(m::model::channels, channelId));
	
	if (w() > BREAK_ARM)
		arm->show();
#ifdef WITH_VST
	if (w() > BREAK_FX)
		fx->show();
#endif
	if (w() > BREAK_MODE_BOX)
		modeBox->show();
	if (w() > BREAK_READ_ACTIONS && ch.hasActions)
		readActions->show();

	packWidgets();
}


/* -------------------------------------------------------------------------- */


void geSampleChannel::changeSize(int H)
{
	geChannel::changeSize(H);

	int Y = y() + (H / 2 - (G_GUI_UNIT / 2));

	status->resize(x(), Y, w(), G_GUI_UNIT);
	modeBox->resize(x(), Y, w(), G_GUI_UNIT);
	readActions->resize(x(), Y, w(), G_GUI_UNIT);
}

}} // giada::v::
