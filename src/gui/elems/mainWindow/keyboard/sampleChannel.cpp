/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#include "sampleChannel.h"
#include "channelMode.h"
#include "channelStatus.h"
#include "column.h"
#include "core/channels/channel.h"
#include "core/channels/samplePlayer.h"
#include "core/clock.h"
#include "core/conf.h"
#include "core/graphics.h"
#include "core/mixer.h"
#include "core/model/model.h"
#include "core/recManager.h"
#include "core/recorder.h"
#include "core/wave.h"
#include "glue/channel.h"
#include "glue/events.h"
#include "glue/io.h"
#include "glue/recorder.h"
#include "glue/storage.h"
#include "gui/dialogs/actionEditor/sampleActionEditor.h"
#include "gui/dialogs/browser/browserLoad.h"
#include "gui/dialogs/browser/browserSave.h"
#include "gui/dialogs/channelNameInput.h"
#include "gui/dialogs/keyGrabber.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/midiIO/midiInputChannel.h"
#include "gui/dialogs/midiIO/midiOutputSampleCh.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/dialogs/warnings.h"
#include "gui/dispatcher.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/statusButton.h"
#include "keyboard.h"
#include "sampleChannelButton.h"
#include "utils/gui.h"
#include <cassert>

extern giada::v::gdMainWindow* G_MainWin;

namespace giada
{
namespace v
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
	EDIT_SAMPLE,
	EDIT_ACTIONS,
	CLEAR_ACTIONS,
	CLEAR_ACTIONS_ALL,
	CLEAR_ACTIONS_VOLUME,
	CLEAR_ACTIONS_START_STOP,
	__END_CLEAR_ACTIONS_SUBMENU__,
	RENAME_CHANNEL,
	CLONE_CHANNEL,
	FREE_CHANNEL,
	DELETE_CHANNEL
};

/* -------------------------------------------------------------------------- */

void menuCallback(Fl_Widget* w, void* v)
{
	const geSampleChannel*  gch  = static_cast<geSampleChannel*>(w);
	const c::channel::Data& data = gch->getData();

	switch ((Menu)(intptr_t)v)
	{
	case Menu::INPUT_MONITOR:
	{
		c::channel::setInputMonitor(data.id, !data.sample->getInputMonitor());
		break;
	}
	case Menu::OVERDUB_PROTECTION:
	{
		c::channel::setOverdubProtection(data.id, !data.sample->getOverdubProtection());
		break;
	}
	case Menu::LOAD_SAMPLE:
	{
		gdWindow* w = new gdBrowserLoad("Browse sample",
		    m::conf::conf.samplePath.c_str(), c::storage::loadSample, data.id);
		u::gui::openSubWindow(G_MainWin, w, WID_FILE_BROWSER);
		break;
	}
	case Menu::EXPORT_SAMPLE:
	{
		gdWindow* w = new gdBrowserSave("Save sample",
		    m::conf::conf.samplePath.c_str(), "", c::storage::saveSample, data.id);
		u::gui::openSubWindow(G_MainWin, w, WID_FILE_BROWSER);
		break;
	}
	case Menu::SETUP_KEYBOARD_INPUT:
	{
		u::gui::openSubWindow(G_MainWin, new gdKeyGrabber(data),
		    WID_KEY_GRABBER);
		break;
	}
	case Menu::SETUP_MIDI_INPUT:
	{
		u::gui::openSubWindow(G_MainWin, new gdMidiInputChannel(data.id),
		    WID_MIDI_INPUT);
		break;
	}
	case Menu::SETUP_MIDI_OUTPUT:
	{
		u::gui::openSubWindow(G_MainWin, new gdMidiOutputSampleCh(data.id),
		    WID_MIDI_OUTPUT);
		break;
	}
	case Menu::EDIT_SAMPLE:
	{
		u::gui::openSubWindow(G_MainWin, new gdSampleEditor(data.id),
		    WID_SAMPLE_EDITOR);
		break;
	}
	case Menu::EDIT_ACTIONS:
	{
		u::gui::openSubWindow(G_MainWin, new gdSampleActionEditor(data.id, m::conf::conf),
		    WID_ACTION_EDITOR);
		break;
	}
	case Menu::CLEAR_ACTIONS:
	case Menu::__END_CLEAR_ACTIONS_SUBMENU__:
		break;
	case Menu::CLEAR_ACTIONS_ALL:
	{
		c::recorder::clearAllActions(data.id);
		break;
	}
	case Menu::CLEAR_ACTIONS_VOLUME:
	{
		c::recorder::clearVolumeActions(data.id);
		break;
	}
	case Menu::CLEAR_ACTIONS_START_STOP:
	{
		c::recorder::clearStartStopActions(data.id);
		break;
	}
	case Menu::CLONE_CHANNEL:
	{
		c::channel::cloneChannel(data.id);
		break;
	}
	case Menu::RENAME_CHANNEL:
	{
		u::gui::openSubWindow(G_MainWin, new gdChannelNameInput(data),
		    WID_SAMPLE_NAME);
		break;
	}
	case Menu::FREE_CHANNEL:
	{
		c::channel::freeChannel(data.id);
		break;
	}
	case Menu::DELETE_CHANNEL:
	{
		c::channel::deleteChannel(data.id);
		break;
	}
	}
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geSampleChannel::geSampleChannel(int X, int Y, int W, int H, c::channel::Data d)
: geChannel(X, Y, W, H, d)
{
#if defined(WITH_VST)
	constexpr int delta = 9 * (G_GUI_UNIT + G_GUI_INNER_MARGIN);
#else
	constexpr int delta = 8 * (G_GUI_UNIT + G_GUI_INNER_MARGIN);
#endif

	playButton  = new geStatusButton(x(), y(), G_GUI_UNIT, G_GUI_UNIT, channelStop_xpm, channelPlay_xpm);
	arm         = new geButton(playButton->x() + playButton->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, G_GUI_UNIT, "", armOff_xpm, armOn_xpm, armDisabled_xpm);
	status      = new geChannelStatus(arm->x() + arm->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, H, m_channel);
	mainButton  = new geSampleChannelButton(status->x() + status->w() + G_GUI_INNER_MARGIN, y(), w() - delta, H, m_channel);
	readActions = new geStatusButton(mainButton->x() + mainButton->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, G_GUI_UNIT, readActionOff_xpm, readActionOn_xpm, readActionDisabled_xpm);
	modeBox     = new geChannelMode(readActions->x() + readActions->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, G_GUI_UNIT, m_channel);
	mute        = new geStatusButton(modeBox->x() + modeBox->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, G_GUI_UNIT, muteOff_xpm, muteOn_xpm);
	solo        = new geStatusButton(mute->x() + mute->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, G_GUI_UNIT, soloOff_xpm, soloOn_xpm);
#if defined(WITH_VST)
	fx  = new geStatusButton(solo->x() + solo->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, G_GUI_UNIT, fxOff_xpm, fxOn_xpm);
	vol = new geDial(fx->x() + fx->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, G_GUI_UNIT);
#else
	vol                 = new geDial(solo->x() + solo->w() + G_GUI_INNER_MARGIN, y(), G_GUI_UNIT, G_GUI_UNIT);
#endif

	end();

	resizable(mainButton);

	playButton->copy_tooltip("Play/stop");
	arm->copy_tooltip("Arm for recording");
	status->copy_tooltip("Progress bar");
	readActions->copy_tooltip("Read actions\n\nToggles playback of pre-recorded "
	                          "actions (key press, key release, ...).");
	modeBox->copy_tooltip("Mode");
	mute->copy_tooltip("Mute");
	solo->copy_tooltip("Solo");
#if defined(WITH_VST)
	fx->copy_tooltip("Plug-ins");
#endif
	vol->copy_tooltip("Volume");

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

	readActions->callback(cb_readActions, (void*)this);

	vol->value(m_channel.volume);
	vol->callback(cb_changeVol, (void*)this);

	size(w(), h()); // Force responsiveness
}

/* -------------------------------------------------------------------------- */

void geSampleChannel::cb_playButton(Fl_Widget* /*w*/, void* p) { ((geSampleChannel*)p)->cb_playButton(); }
void geSampleChannel::cb_openMenu(Fl_Widget* /*w*/, void* p) { ((geSampleChannel*)p)->cb_openMenu(); }
void geSampleChannel::cb_readActions(Fl_Widget* /*w*/, void* p) { ((geSampleChannel*)p)->cb_readActions(); }

/* -------------------------------------------------------------------------- */

void geSampleChannel::cb_playButton()
{
	v::dispatcher::dispatchTouch(*this, playButton->value());
}

/* -------------------------------------------------------------------------- */

void geSampleChannel::cb_openMenu()
{
	/* If you're recording (input or actions) no menu is allowed; you can't do
	anything, especially deallocate the channel. */

	if (m::recManager::isRecording())
		return;

	Fl_Menu_Item rclick_menu[] = {
	    {"Input monitor", 0, menuCallback, (void*)Menu::INPUT_MONITOR,
	        FL_MENU_TOGGLE | (m_channel.sample->getInputMonitor() ? FL_MENU_VALUE : 0)},
	    {"Overdub protection", 0, menuCallback, (void*)Menu::OVERDUB_PROTECTION,
	        FL_MENU_TOGGLE | FL_MENU_DIVIDER | (m_channel.sample->getOverdubProtection() ? FL_MENU_VALUE : 0)},
	    {"Load new sample...", 0, menuCallback, (void*)Menu::LOAD_SAMPLE},
	    {"Export sample to file...", 0, menuCallback, (void*)Menu::EXPORT_SAMPLE},
	    {"Setup keyboard input...", 0, menuCallback, (void*)Menu::SETUP_KEYBOARD_INPUT},
	    {"Setup MIDI input...", 0, menuCallback, (void*)Menu::SETUP_MIDI_INPUT},
	    {"Setup MIDI output...", 0, menuCallback, (void*)Menu::SETUP_MIDI_OUTPUT},
	    {"Edit sample...", 0, menuCallback, (void*)Menu::EDIT_SAMPLE},
	    {"Edit actions...", 0, menuCallback, (void*)Menu::EDIT_ACTIONS},
	    {"Clear actions", 0, menuCallback, (void*)Menu::CLEAR_ACTIONS, FL_SUBMENU},
	    {"All", 0, menuCallback, (void*)Menu::CLEAR_ACTIONS_ALL},
	    {"Volume", 0, menuCallback, (void*)Menu::CLEAR_ACTIONS_VOLUME},
	    {"Start/Stop", 0, menuCallback, (void*)Menu::CLEAR_ACTIONS_START_STOP},
	    {0},
	    {"Rename", 0, menuCallback, (void*)Menu::RENAME_CHANNEL},
	    {"Clone", 0, menuCallback, (void*)Menu::CLONE_CHANNEL},
	    {"Free", 0, menuCallback, (void*)Menu::FREE_CHANNEL},
	    {"Delete", 0, menuCallback, (void*)Menu::DELETE_CHANNEL},
	    {0}};

	if (m_channel.sample->waveId == 0)
	{
		rclick_menu[(int)Menu::EXPORT_SAMPLE].deactivate();
		rclick_menu[(int)Menu::EDIT_SAMPLE].deactivate();
		rclick_menu[(int)Menu::FREE_CHANNEL].deactivate();
		rclick_menu[(int)Menu::RENAME_CHANNEL].deactivate();
	}

	if (!m_channel.hasActions)
		rclick_menu[(int)Menu::CLEAR_ACTIONS].deactivate();

	/* No 'clear start/stop actions' for those channels in loop mode: they cannot
	have start/stop actions. */

	if (m_channel.sample->isLoop)
		rclick_menu[(int)Menu::CLEAR_ACTIONS_START_STOP].deactivate();

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
		if (m_channel.sample->getOverdubProtection())
			arm->deactivate();
		else
			arm->activate();
	}

	if (m_channel.hasActions)
	{
		readActions->activate();
		readActions->setStatus(m_channel.getReadActions());
	}
	else
		readActions->deactivate();
}

/* -------------------------------------------------------------------------- */

void geSampleChannel::draw()
{
	const int ny = y() + (h() / 2) - (G_GUI_UNIT / 2);

	status->resize(status->x(), ny, G_GUI_UNIT, G_GUI_UNIT);
	modeBox->resize(modeBox->x(), ny, G_GUI_UNIT, G_GUI_UNIT);
	readActions->resize(readActions->x(), ny, G_GUI_UNIT, G_GUI_UNIT);

	geChannel::draw();
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

	if (w() > BREAK_ARM)
		arm->show();
#ifdef WITH_VST
	if (w() > BREAK_FX)
		fx->show();
#endif
	if (w() > BREAK_MODE_BOX)
		modeBox->show();
	if (w() > BREAK_READ_ACTIONS)
		readActions->show();

	packWidgets();
}
} // namespace v
} // namespace giada
