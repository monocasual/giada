/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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


#include "../../../../core/mixer.h"
#include "../../../../core/conf.h"
#include "../../../../core/clock.h"
#include "../../../../core/graphics.h"
#include "../../../../core/wave.h"
#include "../../../../core/sampleChannel.h"
#include "../../../../glue/io.h"
#include "../../../../glue/channel.h"
#include "../../../../glue/recorder.h"
#include "../../../../glue/storage.h"
#include "../../../../utils/gui.h"
#include "../../../dialogs/gd_mainWindow.h"
#include "../../../dialogs/gd_keyGrabber.h"
#include "../../../dialogs/sampleEditor.h"
#include "../../../dialogs/channelNameInput.h"
#include "../../../dialogs/gd_actionEditor.h"
#include "../../../dialogs/gd_warnings.h"
#include "../../../dialogs/browser/browserSave.h"
#include "../../../dialogs/browser/browserLoad.h"
#include "../../../dialogs/midiIO/midiOutputSampleCh.h"
#include "../../../dialogs/midiIO/midiInputChannel.h"
#include "../../basics/boxtypes.h"
#include "../../basics/idButton.h"
#include "../../basics/statusButton.h"
#include "../../basics/dial.h"
#include "channelStatus.h"
#include "channelMode.h"
#include "sampleChannelButton.h"
#include "keyboard.h"
#include "column.h"
#include "sampleChannel.h"


extern gdMainWindow* G_MainWin;


using namespace giada;


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
	CLEAR_ACTIONS_MUTE,
	CLEAR_ACTIONS_VOLUME,
	CLEAR_ACTIONS_START_STOP,
	__END_CLEAR_ACTIONS_SUBMENU__,
	RESIZE,
	RESIZE_H1,
	RESIZE_H2,
	RESIZE_H3,
	RESIZE_H4,
	__END_RESIZE_SUBMENU__,
	RENAME_CHANNEL,
	CLONE_CHANNEL,
	FREE_CHANNEL,
	DELETE_CHANNEL
};


/* -------------------------------------------------------------------------- */


void menuCallback(Fl_Widget* w, void* v)
{
	geSampleChannel* gch = static_cast<geSampleChannel*>(w);
	Menu selectedItem = (Menu) (intptr_t) v;

	switch (selectedItem) {
		case Menu::INPUT_MONITOR: {
			glue_toggleInputMonitor(gch->ch);
			break;
		}
		case Menu::LOAD_SAMPLE: {
			gdWindow *w = new gdBrowserLoad(m::conf::browserX, m::conf::browserY,
				m::conf::browserW, m::conf::browserH, "Browse sample",
				m::conf::samplePath.c_str(), glue_loadSample, gch->ch);
			gu_openSubWindow(G_MainWin, w, WID_FILE_BROWSER);
			break;
		}
		case Menu::EXPORT_SAMPLE: {
			gdWindow *w = new gdBrowserSave(m::conf::browserX, m::conf::browserY,
				m::conf::browserW, m::conf::browserH, "Save sample",
				m::conf::samplePath.c_str(), "", glue_saveSample, gch->ch);
			gu_openSubWindow(G_MainWin, w, WID_FILE_BROWSER);
			break;
		}
		case Menu::SETUP_KEYBOARD_INPUT: {
			new gdKeyGrabber(gch->ch); // FIXME - use gu_openSubWindow
			break;
		}
		case Menu::SETUP_MIDI_INPUT: {
			gu_openSubWindow(G_MainWin, new gdMidiInputChannel(gch->ch), 0);
			break;
		}
		case Menu::SETUP_MIDI_OUTPUT: {
			gu_openSubWindow(G_MainWin, new gdMidiOutputSampleCh(static_cast<SampleChannel*>(gch->ch)), 0);
			break;
		}
		case Menu::EDIT_SAMPLE: {
			gu_openSubWindow(G_MainWin, new gdSampleEditor(static_cast<SampleChannel*>(gch->ch)), WID_SAMPLE_EDITOR);
			break;
		}
		case Menu::EDIT_ACTIONS: {
			gu_openSubWindow(G_MainWin, new gdActionEditor(gch->ch), WID_ACTION_EDITOR);
			break;
		}
		case Menu::CLEAR_ACTIONS:
		case Menu::RESIZE:
		case Menu::__END_CLEAR_ACTIONS_SUBMENU__:
		case Menu::__END_RESIZE_SUBMENU__:
			break;
		case Menu::CLEAR_ACTIONS_ALL: {
			c::recorder::clearAllActions(gch);
			break;
		}
		case Menu::CLEAR_ACTIONS_MUTE: {
			c::recorder::clearMuteActions(gch);
			break;
		}
		case Menu::CLEAR_ACTIONS_VOLUME: {
			c::recorder::clearVolumeActions(gch);
			break;
		}
		case Menu::CLEAR_ACTIONS_START_STOP: {
			c::recorder::clearStartStopActions(gch);
			break;
		}
		case Menu::RESIZE_H1: {
			gch->changeSize(G_GUI_CHANNEL_H_1);
			static_cast<geColumn*>(gch->parent())->repositionChannels();
			break;
		}		
		case Menu::RESIZE_H2: {
			gch->changeSize(G_GUI_CHANNEL_H_2);
			static_cast<geColumn*>(gch->parent())->repositionChannels();
			break;
		}		
		case Menu::RESIZE_H3: {
			gch->changeSize(G_GUI_CHANNEL_H_3);
			static_cast<geColumn*>(gch->parent())->repositionChannels();
			break;
		}		
		case Menu::RESIZE_H4: {
			gch->changeSize(G_GUI_CHANNEL_H_4);
			static_cast<geColumn*>(gch->parent())->repositionChannels();
			break;
		}
		case Menu::CLONE_CHANNEL: {
			glue_cloneChannel(gch->ch);
			break;
		}
		case Menu::RENAME_CHANNEL: {
			gu_openSubWindow(G_MainWin, new gdChannelNameInput(gch->ch), WID_SAMPLE_NAME);
			break;
		}
		case Menu::FREE_CHANNEL: {
			glue_freeChannel(gch->ch);
			break;
		}
		case Menu::DELETE_CHANNEL: {
			glue_deleteChannel(gch->ch);
			break;
		}
	}
}

}; // {namespace}


/* -------------------------------------------------------------------------- */


geSampleChannel::geSampleChannel(int X, int Y, int W, int H, SampleChannel* ch)
	: geChannel(X, Y, W, H, CHANNEL_SAMPLE, ch)
{
	begin();

	button      = new geIdButton(x(), y(), G_GUI_UNIT, G_GUI_UNIT, "", channelStop_xpm, channelPlay_xpm);
	arm         = new geButton(button->x()+button->w()+4, y(), G_GUI_UNIT, G_GUI_UNIT, "", armOff_xpm, armOn_xpm);
	status      = new geChannelStatus(arm->x()+arm->w()+4, y(), G_GUI_UNIT, H, ch);
	mainButton  = new geSampleChannelButton(status->x()+status->w()+4, y(), G_GUI_UNIT, H, "-- no sample --");
	readActions = new geButton(mainButton->x()+mainButton->w()+4, y(), G_GUI_UNIT, G_GUI_UNIT, "", readActionOff_xpm, readActionOn_xpm);
	modeBox     = new geChannelMode(readActions->x()+readActions->w()+4, y(), G_GUI_UNIT, G_GUI_UNIT, ch);
	mute        = new geButton(modeBox->x()+modeBox->w()+4, y(), G_GUI_UNIT, G_GUI_UNIT, "", muteOff_xpm, muteOn_xpm);
	solo        = new geButton(mute->x()+mute->w()+4, y(), G_GUI_UNIT, G_GUI_UNIT, "", soloOff_xpm, soloOn_xpm);
#ifdef WITH_VST
	fx          = new geStatusButton(solo->x()+solo->w()+4, y(), G_GUI_UNIT, G_GUI_UNIT, fxOff_xpm, fxOn_xpm);
	vol         = new geDial(fx->x()+fx->w()+4, y(), G_GUI_UNIT, G_GUI_UNIT);
#else
	vol         = new geDial(solo->x()+solo->w()+4, y(), G_GUI_UNIT, G_GUI_UNIT);
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

	readActions->type(FL_TOGGLE_BUTTON);
	readActions->value(ch->readActions);
	readActions->callback(cb_readActions, (void*)this);

	vol->callback(cb_changeVol, (void*)this);

	ch->guiChannel = this;

	changeSize(H);  // Update size dynamically
}


/* -------------------------------------------------------------------------- */


void geSampleChannel::cb_button      (Fl_Widget* v, void* p) { ((geSampleChannel*)p)->__cb_button(); }
void geSampleChannel::cb_openMenu    (Fl_Widget* v, void* p) { ((geSampleChannel*)p)->__cb_openMenu(); }
void geSampleChannel::cb_readActions (Fl_Widget* v, void* p) { ((geSampleChannel*)p)->__cb_readActions(); }


/* -------------------------------------------------------------------------- */


void geSampleChannel::__cb_button()
{
	if (button->value())    // pushed
		glue_keyPress(ch, Fl::event_ctrl(), Fl::event_shift());
	else                    // released
		glue_keyRelease(ch, Fl::event_ctrl(), Fl::event_shift());
}


/* -------------------------------------------------------------------------- */


void geSampleChannel::__cb_openMenu()
{
	/* If you're recording (input or actions) no menu is allowed; you can't do
	anything, especially deallocate the channel */

	if (m::mixer::recording || m::recorder::active)
		return;

	Fl_Menu_Item rclick_menu[] = {
		{"Input monitor",            0, menuCallback, (void*) Menu::INPUT_MONITOR,
			FL_MENU_TOGGLE | FL_MENU_DIVIDER | (static_cast<SampleChannel*>(ch)->inputMonitor ? FL_MENU_VALUE : 0)},
		{"Load new sample...",       0, menuCallback, (void*) Menu::LOAD_SAMPLE},
		{"Export sample to file...", 0, menuCallback, (void*) Menu::EXPORT_SAMPLE},
		{"Setup keyboard input...",  0, menuCallback, (void*) Menu::SETUP_KEYBOARD_INPUT},
		{"Setup MIDI input...",      0, menuCallback, (void*) Menu::SETUP_MIDI_INPUT},
		{"Setup MIDI output...",     0, menuCallback, (void*) Menu::SETUP_MIDI_OUTPUT},
		{"Edit sample...",           0, menuCallback, (void*) Menu::EDIT_SAMPLE},
		{"Edit actions...",          0, menuCallback, (void*) Menu::EDIT_ACTIONS},
		{"Clear actions",            0, menuCallback, (void*) Menu::CLEAR_ACTIONS, FL_SUBMENU},
			{"All",        0, menuCallback, (void*) Menu::CLEAR_ACTIONS_ALL},
			{"Mute",       0, menuCallback, (void*) Menu::CLEAR_ACTIONS_MUTE},
			{"Volume",     0, menuCallback, (void*) Menu::CLEAR_ACTIONS_VOLUME},
			{"Start/Stop", 0, menuCallback, (void*) Menu::CLEAR_ACTIONS_START_STOP},
			{0},
		{"Resize",    0, menuCallback, (void*) Menu::RESIZE, FL_SUBMENU},
			{"Normal",  0, menuCallback, (void*) Menu::RESIZE_H1},
			{"Medium",  0, menuCallback, (void*) Menu::RESIZE_H2},
			{"Large",   0, menuCallback, (void*) Menu::RESIZE_H3},
			{"X-Large", 0, menuCallback, (void*) Menu::RESIZE_H4},
			{0},
		{"Rename channel", 0, menuCallback, (void*) Menu::RENAME_CHANNEL},
		{"Clone channel",  0, menuCallback, (void*) Menu::CLONE_CHANNEL},
		{"Free channel",   0, menuCallback, (void*) Menu::FREE_CHANNEL},
		{"Delete channel", 0, menuCallback, (void*) Menu::DELETE_CHANNEL},
		{0}
	};

	if (ch->status & (STATUS_EMPTY | STATUS_MISSING)) {
		rclick_menu[(int) Menu::EXPORT_SAMPLE].deactivate();
		rclick_menu[(int) Menu::EDIT_SAMPLE].deactivate();
		rclick_menu[(int) Menu::FREE_CHANNEL].deactivate();
		rclick_menu[(int) Menu::RENAME_CHANNEL].deactivate();
	}

	if (!ch->hasActions)
		rclick_menu[(int) Menu::CLEAR_ACTIONS].deactivate();


	/* No 'clear start/stop actions' for those channels in loop mode: they cannot
	have start/stop actions. */

	if (static_cast<SampleChannel*>(ch)->mode & LOOP_ANY)
		rclick_menu[(int) Menu::CLEAR_ACTIONS_START_STOP].deactivate();

	Fl_Menu_Button* b = new Fl_Menu_Button(0, 0, 100, 50);
	b->box(G_CUSTOM_BORDER_BOX);
	b->textsize(G_GUI_FONT_SIZE_BASE);
	b->textcolor(G_COLOR_LIGHT_2);
	b->color(G_COLOR_GREY_2);

	const Fl_Menu_Item* m = rclick_menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, b);
	if (m)
		m->do_callback(this, m->user_data());
	return;
}


/* -------------------------------------------------------------------------- */


void geSampleChannel::__cb_readActions()
{
	glue_toggleReadingRecs(static_cast<SampleChannel*>(ch));
}


/* -------------------------------------------------------------------------- */


void geSampleChannel::refresh()
{
	if (!mainButton->visible()) // mainButton invisible? status too (see below)
		return;

	setColorsByStatus(ch->status, ch->recStatus);

	if (static_cast<SampleChannel*>(ch)->wave != nullptr) {
		if (m::mixer::recording && ch->isArmed())
			mainButton->setInputRecordMode();
		if (m::recorder::active) {
			if (m::recorder::canRec(ch, m::clock::isRunning(), m::mixer::recording))
				mainButton->setActionRecordMode();
		}
		status->redraw(); // status invisible? sampleButton too (see below)
	}
	mainButton->redraw();
}


/* -------------------------------------------------------------------------- */


void geSampleChannel::reset()
{
	hideActionButton();
	mainButton->setDefaultMode("-- no sample --");
	mainButton->redraw();
	status->redraw();
}


/* -------------------------------------------------------------------------- */


void geSampleChannel::update()
{
	const SampleChannel* sch = static_cast<const SampleChannel*>(ch);

	switch (sch->status) {
		case STATUS_EMPTY:
			mainButton->label("-- no sample --");
			break;
		case STATUS_MISSING:
		case STATUS_WRONG:
			mainButton->label("* file not found! *");
			break;
		default:
			if (sch->getName().empty())
				mainButton->label(sch->wave->getBasename(false).c_str());
			else
				mainButton->label(sch->getName().c_str());
			break;
	}

	/* Update channels. If you load a patch with recorded actions, the 'R' button 
	must be shown. Moreover if the actions are active, the 'R' button must be 
	activated accordingly. */

	if (sch->hasActions)
		showActionButton();
	else
		hideActionButton();

	modeBox->value(sch->mode);
	modeBox->redraw();

	vol->value(sch->volume);
	mute->value(sch->mute);
	solo->value(sch->solo);

	mainButton->setKey(sch->key);

	arm->value(sch->isArmed());

#ifdef WITH_VST
	fx->status = sch->plugins.size() > 0;
	fx->redraw();
#endif
}


/* -------------------------------------------------------------------------- */


void geSampleChannel::showActionButton()
{
	readActions->value(static_cast<SampleChannel*>(ch)->readActions);
	readActions->show();
	packWidgets();
	redraw();
}


/* -------------------------------------------------------------------------- */


void geSampleChannel::hideActionButton()
{
	readActions->hide();
	packWidgets();
	redraw();
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
	if (w() > BREAK_READ_ACTIONS && ch->hasActions)
		readActions->show();

	packWidgets();
}


void geSampleChannel::changeSize(int H)
{
	geChannel::changeSize(H);

	int Y = y() + (H / 2 - (G_GUI_UNIT / 2));

	status->resize(x(), Y, w(), G_GUI_UNIT);
	modeBox->resize(x(), Y, w(), G_GUI_UNIT);
	readActions->resize(x(), Y, w(), G_GUI_UNIT);
}