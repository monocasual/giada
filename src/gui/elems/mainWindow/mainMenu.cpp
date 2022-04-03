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

#include "gui/elems/mainWindow/mainMenu.h"
#include "core/const.h"
#include "core/patch.h"
#include "glue/layout.h"
#include "glue/main.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/button.h"
#include "gui/ui.h"
#include "keyboard/keyboard.h"
#include "utils/gui.h"
#include <FL/Fl_Menu_Button.H>

extern giada::v::Ui g_ui;

namespace giada::v
{
namespace
{
enum class FileMenu
{
	OPEN_PROJECT = 0,
	SAVE_PROJECT,
	CLOSE_PROJECT,
#ifdef G_DEBUG_MODE
	DEBUG_STATS,
#endif
	QUIT
};

void fileMenuCallback(Fl_Widget* /*w*/, void* v)
{
	switch ((FileMenu)(intptr_t)v)
	{
	case FileMenu::OPEN_PROJECT:
		c::layout::openBrowserForProjectLoad();
		break;
	case FileMenu::SAVE_PROJECT:
		c::layout::openBrowserForProjectSave();
		break;
	case FileMenu::CLOSE_PROJECT:
		c::main::closeProject();
		break;
#ifdef G_DEBUG_MODE
	case FileMenu::DEBUG_STATS:
		c::main::printDebugInfo();
		break;
#endif
	case FileMenu::QUIT:
		c::main::quitGiada();
		break;
	}
}

/* -------------------------------------------------------------------------- */

enum class EditMenu
{
	FREE_SAMPLE_CHANNELS = 0,
	CLEAR_ALL_ACTIONS,
	SETUP_MIDI_INPUT
};

void editMenuCallback(Fl_Widget* /*w*/, void* v)
{
	switch ((EditMenu)(intptr_t)v)
	{
	case EditMenu::FREE_SAMPLE_CHANNELS:
		c::main::clearAllSamples();
		break;
	case EditMenu::CLEAR_ALL_ACTIONS:
		c::main::clearAllActions();
		break;
	case EditMenu::SETUP_MIDI_INPUT:
		c::layout::openMasterMidiInputWindow();
		break;
	}
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geMainMenu::geMainMenu()
: geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN)
{
	geButton* file   = new geButton(g_ui.langMapper.get(LangMap::MAIN_MENU_FILE));
	geButton* edit   = new geButton(g_ui.langMapper.get(LangMap::MAIN_MENU_EDIT));
	geButton* config = new geButton(g_ui.langMapper.get(LangMap::MAIN_MENU_CONFIG));
	geButton* about  = new geButton(g_ui.langMapper.get(LangMap::MAIN_MENU_ABOUT));
	add(file, 80);
	add(edit, 80);
	add(config, 80);
	add(about, 80);
	end();

	file->onClick   = [this]() { cb_file(); };
	edit->onClick   = [this]() { cb_edit(); };
	about->onClick  = []() { c::layout::openAboutWindow(); };
	config->onClick = []() { c::layout::openConfigWindow(); };
}

/* -------------------------------------------------------------------------- */

void geMainMenu::cb_file()
{
	Fl_Menu_Item menu[] = {
	    u::gui::makeMenuItem(g_ui.langMapper.get(LangMap::MAIN_MENU_FILE_OPENPROJECT), fileMenuCallback, (void*)FileMenu::OPEN_PROJECT),
	    u::gui::makeMenuItem(g_ui.langMapper.get(LangMap::MAIN_MENU_FILE_SAVEPROJECT), fileMenuCallback, (void*)FileMenu::SAVE_PROJECT),
	    u::gui::makeMenuItem(g_ui.langMapper.get(LangMap::MAIN_MENU_FILE_CLOSEPROJECT), fileMenuCallback, (void*)FileMenu::CLOSE_PROJECT),
#ifdef G_DEBUG_MODE
	    u::gui::makeMenuItem("Debug stats", fileMenuCallback, (void*)FileMenu::DEBUG_STATS),
#endif
	    u::gui::makeMenuItem(g_ui.langMapper.get(LangMap::MAIN_MENU_FILE_QUIT), fileMenuCallback, (void*)FileMenu::QUIT),
	    {}};

	Fl_Menu_Button b(0, 0, 100, 50);
	b.box(G_CUSTOM_BORDER_BOX);
	b.textsize(G_GUI_FONT_SIZE_BASE);
	b.textcolor(G_COLOR_LIGHT_2);
	b.color(G_COLOR_GREY_2);

	const Fl_Menu_Item* m = menu->popup(Fl::event_x(), Fl::event_y(), 0, 0, &b);
	if (m != nullptr)
		m->do_callback(this, m->user_data());
}

/* -------------------------------------------------------------------------- */

void geMainMenu::cb_edit()
{
	c::main::MainMenu menu = c::main::getMainMenu();

	Fl_Menu_Item menuItem[] = {
	    u::gui::makeMenuItem(g_ui.langMapper.get(LangMap::MAIN_MENU_EDIT_FREEALLSAMPLES), editMenuCallback, (void*)EditMenu::FREE_SAMPLE_CHANNELS),
	    u::gui::makeMenuItem(g_ui.langMapper.get(LangMap::MAIN_MENU_EDIT_CLEARALLACTIONS), editMenuCallback, (void*)EditMenu::CLEAR_ALL_ACTIONS),
	    u::gui::makeMenuItem(g_ui.langMapper.get(LangMap::MAIN_MENU_EDIT_SETUPMIDIINPUT), editMenuCallback, (void*)EditMenu::SETUP_MIDI_INPUT),
	    {}};

	menuItem[(int)EditMenu::FREE_SAMPLE_CHANNELS].deactivate();
	menuItem[(int)EditMenu::CLEAR_ALL_ACTIONS].deactivate();

	if (menu.hasAudioData)
		menuItem[(int)EditMenu::FREE_SAMPLE_CHANNELS].activate();
	if (menu.hasActions)
		menuItem[(int)EditMenu::CLEAR_ALL_ACTIONS].activate();

	Fl_Menu_Button b(0, 0, 100, 50);
	b.box(G_CUSTOM_BORDER_BOX);
	b.textsize(G_GUI_FONT_SIZE_BASE);
	b.textcolor(G_COLOR_LIGHT_2);
	b.color(G_COLOR_GREY_2);

	const Fl_Menu_Item* m = menuItem->popup(Fl::event_x(), Fl::event_y(), 0, 0, &b);
	if (m != nullptr)
		m->do_callback(this, m->user_data());
}
} // namespace giada::v