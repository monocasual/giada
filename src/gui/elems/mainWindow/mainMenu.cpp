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
#include <FL/Fl_Menu_Button.H>
#include "core/model/model.h"
#include "core/const.h"
#include "core/mixer.h"
#include "core/mixerHandler.h"
#include "core/conf.h"
#include "core/patch.h"
#include "utils/gui.h"
#include "glue/storage.h"
#include "glue/main.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/button.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/about.h"
#include "gui/dialogs/config.h"
#include "gui/dialogs/warnings.h"
#include "gui/dialogs/browser/browserLoad.h"
#include "gui/dialogs/browser/browserSave.h"
#include "gui/dialogs/midiIO/midiInputMaster.h"
#include "keyboard/keyboard.h"
#include "mainMenu.h"


extern giada::v::gdMainWindow* G_MainWin;


namespace giada {
namespace v
{
geMainMenu::geMainMenu(int x, int y)
: gePack(x, y, Direction::HORIZONTAL)
{
	file   = new geButton(0, 0, 70, 21, "file");
	edit   = new geButton(0, 0, 70, 21, "edit");
	config = new geButton(0, 0, 70, 21, "config");
	about  = new geButton(0, 0, 70, 21, "about");
	add(file);
	add(edit);
	add(config);
	add(about);

	resizable(nullptr);   // don't resize any widget

	file->callback(cb_file, (void*)this);
	edit->callback(cb_edit, (void*)this);

	about->callback([](Fl_Widget* /*w*/, void* /*v*/) { 
		u::gui::openSubWindow(G_MainWin, new gdAbout(), WID_ABOUT);
	});
	config->callback([](Fl_Widget* /*w*/, void* /*v*/) { 
		u::gui::openSubWindow(G_MainWin, new gdConfig(400, 370), WID_CONFIG);
	});
}


/* -------------------------------------------------------------------------- */


void geMainMenu::cb_file(Fl_Widget* /*w*/, void* p) { ((geMainMenu*)p)->cb_file(); }
void geMainMenu::cb_edit(Fl_Widget* /*w*/, void* p) { ((geMainMenu*)p)->cb_edit(); }


/* -------------------------------------------------------------------------- */


void geMainMenu::cb_file()
{
	using namespace giada::m;

	/* An Fl_Menu_Button is made of many Fl_Menu_Item */

	Fl_Menu_Item menu[] = {
		{"Open project..."},
		{"Save project..."},
		{"Close project"},
#ifndef NDEBUG
		{"Debug stats"},
#endif
		{"Quit Giada"},
		{0}
	};

	Fl_Menu_Button b(0, 0, 100, 50);
	b.box(G_CUSTOM_BORDER_BOX);
	b.textsize(G_GUI_FONT_SIZE_BASE);
	b.textcolor(G_COLOR_LIGHT_2);
	b.color(G_COLOR_GREY_2);

	const Fl_Menu_Item* m = menu->popup(Fl::event_x(),	Fl::event_y(), 0, 0, &b);
	if (!m) return;

	if (strcmp(m->label(), "Open project...") == 0) {
		gdWindow* childWin = new gdBrowserLoad("Open project", 
			conf::conf.patchPath, c::storage::loadProject, 0);
		u::gui::openSubWindow(G_MainWin, childWin, WID_FILE_BROWSER);
	}
	else
	if (strcmp(m->label(), "Save project...") == 0) {
		gdWindow* childWin = new gdBrowserSave("Save project", conf::conf.patchPath, 
			patch::patch.name, c::storage::saveProject, 0);
		u::gui::openSubWindow(G_MainWin, childWin, WID_FILE_BROWSER);
	}
	else
	if (strcmp(m->label(), "Close project") == 0) {
		c::main::closeProject();
	}
#ifndef NDEBUG
	else
	if (strcmp(m->label(), "Debug stats") == 0) {
		m::model::debug();
	}
#endif
	else
	if (strcmp(m->label(), "Quit Giada") == 0) {
		G_MainWin->do_callback();
	}
}


/* -------------------------------------------------------------------------- */


void geMainMenu::cb_edit()
{
	Fl_Menu_Item menu[] = {
		{"Free all Sample channels"},
		{"Clear all actions"},
		{"Setup global MIDI input..."},
		{0}
	};

	menu[0].deactivate();
	menu[1].deactivate();

	if (m::mh::hasAudioData()) menu[0].activate();
	if (m::mh::hasActions())   menu[1].activate();

	Fl_Menu_Button b(0, 0, 100, 50);
	b.box(G_CUSTOM_BORDER_BOX);
	b.textsize(G_GUI_FONT_SIZE_BASE);
	b.textcolor(G_COLOR_LIGHT_2);
	b.color(G_COLOR_GREY_2);

	const Fl_Menu_Item* m = menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, &b);
	if (!m) return;

	if (strcmp(m->label(), "Free all Sample channels") == 0) 
		c::main::clearAllSamples();
	else
	if (strcmp(m->label(), "Clear all actions") == 0) 
		c::main::clearAllActions();
	else
	if (strcmp(m->label(), "Setup global MIDI input...") == 0) 
		u::gui::openSubWindow(G_MainWin, new gdMidiInputMaster(), WID_MIDI_INPUT);
}

}} // giada::v::
