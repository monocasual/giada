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

#include "gui/elems/mainWindow/mainMenu.h"
#include "core/const.h"
#include "core/patch.h"
#include "glue/layout.h"
#include "glue/main.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/menu.h"
#include "gui/elems/basics/textButton.h"
#include "gui/ui.h"
#include "keyboard/keyboard.h"
#include "utils/gui.h"

extern giada::v::Ui* g_ui;

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

enum class EditMenu
{
	FREE_SAMPLE_CHANNELS = 0,
	CLEAR_ALL_ACTIONS,
	SETUP_MIDI_INPUT
};
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geMainMenu::geMainMenu()
: geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN)
{
	geTextButton* file   = new geTextButton(g_ui->getI18Text(LangMap::MAIN_MENU_FILE));
	geTextButton* edit   = new geTextButton(g_ui->getI18Text(LangMap::MAIN_MENU_EDIT));
	geTextButton* config = new geTextButton(g_ui->getI18Text(LangMap::MAIN_MENU_CONFIG));
	geTextButton* about  = new geTextButton(g_ui->getI18Text(LangMap::MAIN_MENU_ABOUT));
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
	geMenu menu;

	menu.addItem((ID)FileMenu::OPEN_PROJECT, g_ui->getI18Text(LangMap::MAIN_MENU_FILE_OPENPROJECT));
	menu.addItem((ID)FileMenu::SAVE_PROJECT, g_ui->getI18Text(LangMap::MAIN_MENU_FILE_SAVEPROJECT));
	menu.addItem((ID)FileMenu::CLOSE_PROJECT, g_ui->getI18Text(LangMap::MAIN_MENU_FILE_CLOSEPROJECT));
#ifdef G_DEBUG_MODE
	menu.addItem((ID)FileMenu::DEBUG_STATS, "Debug stats");
#endif
	menu.addItem((ID)FileMenu::QUIT, g_ui->getI18Text(LangMap::MAIN_MENU_FILE_QUIT));

	menu.onSelect = [](ID id) {
		switch (static_cast<FileMenu>(id))
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
	};

	menu.popup();
}

/* -------------------------------------------------------------------------- */

void geMainMenu::cb_edit()
{
	c::main::MainMenu menuData = c::main::getMainMenu();
	geMenu            menu;

	menu.addItem((ID)EditMenu::FREE_SAMPLE_CHANNELS, g_ui->getI18Text(LangMap::MAIN_MENU_EDIT_FREEALLSAMPLES));
	menu.addItem((ID)EditMenu::CLEAR_ALL_ACTIONS, g_ui->getI18Text(LangMap::MAIN_MENU_EDIT_CLEARALLACTIONS));
	menu.addItem((ID)EditMenu::SETUP_MIDI_INPUT, g_ui->getI18Text(LangMap::MAIN_MENU_EDIT_SETUPMIDIINPUT));

	menu.setEnabled((ID)EditMenu::FREE_SAMPLE_CHANNELS, menuData.hasAudioData);
	menu.setEnabled((ID)EditMenu::CLEAR_ALL_ACTIONS, menuData.hasActions);

	menu.onSelect = [](ID id) {
		switch (static_cast<EditMenu>(id))
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
	};

	menu.popup();
}
} // namespace giada::v