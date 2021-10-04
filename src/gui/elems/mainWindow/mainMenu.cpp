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

#include "mainMenu.h"
#include "core/const.h"
#include "core/patch.h"
#include "glue/layout.h"
#include "glue/main.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/button.h"
#include "keyboard/keyboard.h"
#include <FL/Fl_Menu_Button.H>

namespace giada::v
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

	resizable(nullptr); // don't resize any widget

	file->callback(cb_file, (void*)this);
	edit->callback(cb_edit, (void*)this);

	about->callback([](Fl_Widget* /*w*/, void* /*v*/) {
		c::layout::openAboutWindow();
	});
	config->callback([](Fl_Widget* /*w*/, void* /*v*/) {
		c::layout::openConfigWindow();
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
	    {0}};

	Fl_Menu_Button b(0, 0, 100, 50);
	b.box(G_CUSTOM_BORDER_BOX);
	b.textsize(G_GUI_FONT_SIZE_BASE);
	b.textcolor(G_COLOR_LIGHT_2);
	b.color(G_COLOR_GREY_2);

	const Fl_Menu_Item* m = menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, &b);
	if (!m)
		return;

	if (strcmp(m->label(), "Open project...") == 0)
	{
		c::layout::openBrowserForProjectLoad();
	}
	else if (strcmp(m->label(), "Save project...") == 0)
	{
		c::layout::openBrowserForProjectSave();
	}
	else if (strcmp(m->label(), "Close project") == 0)
	{
		c::main::closeProject();
	}
#ifndef NDEBUG
	else if (strcmp(m->label(), "Debug stats") == 0)
	{
		c::main::printDebugInfo();
	}
#endif
	else if (strcmp(m->label(), "Quit Giada") == 0)
	{
		c::main::quitGiada();
	}
}

/* -------------------------------------------------------------------------- */

void geMainMenu::cb_edit()
{
	c::main::MainMenu menu = c::main::getMainMenu();

	Fl_Menu_Item menuItem[] = {
	    {"Free all Sample channels"},
	    {"Clear all actions"},
	    {"Setup global MIDI input..."},
	    {0}};

	menuItem[0].deactivate();
	menuItem[1].deactivate();

	if (menu.hasAudioData)
		menuItem[0].activate();
	if (menu.hasActions)
		menuItem[1].activate();

	Fl_Menu_Button b(0, 0, 100, 50);
	b.box(G_CUSTOM_BORDER_BOX);
	b.textsize(G_GUI_FONT_SIZE_BASE);
	b.textcolor(G_COLOR_LIGHT_2);
	b.color(G_COLOR_GREY_2);

	const Fl_Menu_Item* m = menuItem->popup(Fl::event_x(), Fl::event_y(), 0, 0, &b);
	if (!m)
		return;

	if (strcmp(m->label(), "Free all Sample channels") == 0)
		c::main::clearAllSamples();
	else if (strcmp(m->label(), "Clear all actions") == 0)
		c::main::clearAllActions();
	else if (strcmp(m->label(), "Setup global MIDI input...") == 0)
		c::layout::openMasterMidiInputWindow();
}
} // namespace giada::v
