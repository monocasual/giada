/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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
#include "core/patch.h"
#include "glue/layout.h"
#include "glue/main.h"
#include "gui/const.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/menu.h"
#include "gui/elems/basics/textButton.h"
#include "gui/ui.h"
#include "keyboard/keyboard.h"
#include "utils/gui.h"
#include <FL/Fl_Menu_Item.H>
#ifdef G_OS_MAC
#include <FL/platform.H>
#endif

extern giada::v::Ui* g_ui;

namespace giada::v
{
namespace
{
template <typename Callback>
Fl_Menu_Item makeMenuItem_(const char* l, Callback&& cb, int flags = 0)
{
	return {g_ui->getI18Text(l), 0, cb, nullptr, flags, 0, 0, 0, 0};
}

/* -------------------------------------------------------------------------- */

Fl_Menu_Item beginSubMenu_(const char* l)
{
	return {g_ui->getI18Text(l), 0, nullptr, nullptr, FL_SUBMENU, 0, 0, 0, 0};
}

/* -------------------------------------------------------------------------- */

Fl_Menu_Item endSubMenu_()
{
	return {nullptr, 0, nullptr, nullptr, 0, 0, 0, 0, 0};
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geMainMenu::geMainMenu()
: Fl_Sys_Menu_Bar(0, 0, 0, 0)
{
	textsize(G_GUI_FONT_SIZE_BASE);
	textcolor(G_COLOR_LIGHT_2);

	Fl_Menu_Item popup[] = {
	    beginSubMenu_(LangMap::MAIN_MENU_FILE),
	    makeMenuItem_(LangMap::MAIN_MENU_FILE_OPENPROJECT, [](Fl_Widget*, void*)
	{ c::layout::openBrowserForProjectLoad(); }),
	    makeMenuItem_(LangMap::MAIN_MENU_FILE_SAVEPROJECT, [](Fl_Widget*, void*)
	{ c::layout::openBrowserForProjectSave(); }),
	    makeMenuItem_(LangMap::MAIN_MENU_FILE_CLOSEPROJECT, [](Fl_Widget*, void*)
	{ c::main::closeProject(); }),
#ifdef G_DEBUG_MODE
	    makeMenuItem_(LangMap::MAIN_MENU_FILE_DEBUGSTATS, [](Fl_Widget*, void*)
	{ c::main::printDebugInfo(); }),
#endif
	    makeMenuItem_(LangMap::MAIN_MENU_FILE_QUIT, [](Fl_Widget*, void*)
	{ c::main::quitGiada(); }),
	    endSubMenu_(),
	    beginSubMenu_(LangMap::MAIN_MENU_EDIT),
	    makeMenuItem_(LangMap::MAIN_MENU_EDIT_FREEALLSAMPLES, [](Fl_Widget*, void*)
	{ c::main::clearAllSamples(); }),
	    makeMenuItem_(LangMap::MAIN_MENU_EDIT_CLEARALLACTIONS, [](Fl_Widget*, void*)
	{ c::main::clearAllActions(); }),
	    makeMenuItem_(LangMap::MAIN_MENU_EDIT_SETUPMIDIINPUT, [](Fl_Widget*, void*)
	{ c::layout::openMasterMidiInputWindow(); }),
	    endSubMenu_(),
#ifndef G_OS_MAC // "Config" and "About" are treated differently on macOS menu
	    makeMenuItem_(LangMap::MAIN_MENU_CONFIG, [](Fl_Widget*, void*)
	{ c::layout::openConfigWindow(); }),
	    makeMenuItem_(LangMap::MAIN_MENU_ABOUT, [](Fl_Widget*, void*)
	{ c::layout::openAboutWindow(); }),
#endif
	    endSubMenu_()};

	copy(popup);

#ifdef G_OS_MAC
	static const Fl_Menu_Item macOSextra[] = {
	    makeMenuItem_(LangMap::MAIN_MENU_CONFIG, [](Fl_Widget*, void*)
	{ c::layout::openConfigWindow(); }),
	    endSubMenu_(),
	    endSubMenu_()};
	Fl_Mac_App_Menu::custom_application_menu_items(macOSextra);
	Fl_Sys_Menu_Bar::about([](Fl_Widget*, void*)
	{ c::layout::openAboutWindow(); }, nullptr);
#endif
}
} // namespace giada::v