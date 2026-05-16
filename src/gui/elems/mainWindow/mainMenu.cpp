/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/gui/elems/mainWindow/mainMenu.h"
#include "src/glue/layout.h"
#include "src/glue/main.h"
#include "src/gui/const.h"
#include "src/gui/elems/basics/boxtypes.h"
#include "src/gui/elems/basics/menu.h"
#include "src/gui/elems/basics/textButton.h"
#include "src/gui/elems/mainWindow/keyboard/keyboard.h"
#include "src/gui/ui.h"
#include "src/utils/gui.h"
#include <FL/Fl_Menu_Item.H>
#if G_OS_MAC
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

template <typename Callback>
Fl_Menu_Item makeMenuItemText_(const char* l, Callback&& cb, int flags = 0)
{
	return {l, 0, cb, nullptr, flags, 0, 0, 0, 0};
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
#if G_DEBUG_MODE
	    makeMenuItem_(LangMap::MAIN_MENU_FILE_DEBUGSTATS, [](Fl_Widget*, void*)
	{ c::main::printDebugInfo(); }),
#endif
	    makeMenuItem_(LangMap::MAIN_MENU_FILE_QUIT, [](Fl_Widget*, void*)
	{ c::main::quitGiada(); }),
	    endSubMenu_(),
	    beginSubMenu_(LangMap::MAIN_MENU_EDIT),
	    beginSubMenu_(LangMap::MAIN_MENU_EDIT_FREEALLSAMPLES),
	    makeMenuItem_(LangMap::MAIN_MENU_EDIT_INTHISSCENE, [](Fl_Widget*, void*)
	{ c::main::clearAllSamples(/*allScenes=*/false); }),
	    makeMenuItem_(LangMap::MAIN_MENU_EDIT_INALLSCENES, [](Fl_Widget*, void*)
	{ c::main::clearAllSamples(/*allScenes=*/true); }),
	    endSubMenu_(),
	    beginSubMenu_(LangMap::MAIN_MENU_EDIT_CLEARALLACTIONS),
	    makeMenuItem_(LangMap::MAIN_MENU_EDIT_INTHISSCENE, [](Fl_Widget*, void*)
	{ c::main::clearAllActions(/*allScenes=*/false); }),
	    makeMenuItem_(LangMap::MAIN_MENU_EDIT_INALLSCENES, [](Fl_Widget*, void*)
	{ c::main::clearAllActions(/*allScenes=*/true); }),
	    endSubMenu_(),
	    beginSubMenu_(LangMap::MAIN_MENU_EDIT_COPYCURRENTSCENE),
	    // TODO - for some crappy reason we can't pass fmt::format().c_str() to this crappy menu system. Fuck it...
	    makeMenuItemText_("Scene 1", [](Fl_Widget*, void*)
	{ c::main::copyCurrentScene(Scene{0}); }),
	    makeMenuItemText_("Scene 2", [](Fl_Widget*, void*)
	{ c::main::copyCurrentScene(Scene{1}); }),
	    makeMenuItemText_("Scene 3", [](Fl_Widget*, void*)
	{ c::main::copyCurrentScene(Scene{2}); }),
	    makeMenuItemText_("Scene 4", [](Fl_Widget*, void*)
	{ c::main::copyCurrentScene(Scene{3}); }),
	    makeMenuItemText_("Scene 5", [](Fl_Widget*, void*)
	{ c::main::copyCurrentScene(Scene{4}); }),
	    makeMenuItemText_("Scene 6", [](Fl_Widget*, void*)
	{ c::main::copyCurrentScene(Scene{5}); }),
	    makeMenuItemText_("Scene 7", [](Fl_Widget*, void*)
	{ c::main::copyCurrentScene(Scene{6}); }),
	    makeMenuItemText_("Scene 8", [](Fl_Widget*, void*)
	{ c::main::copyCurrentScene(Scene{7}); }),
	    endSubMenu_(),
	    makeMenuItem_(LangMap::MAIN_MENU_EDIT_SETUPMIDIINPUT, [](Fl_Widget*, void*)
	{ c::layout::openMasterMidiInputWindow(); }),
	    endSubMenu_(),
#if !G_OS_MAC // "Config" and "About" are treated differently on macOS menu
	    makeMenuItem_(LangMap::MAIN_MENU_CONFIG, [](Fl_Widget*, void*)
	{ c::layout::openConfigWindow(); }),
	    makeMenuItem_(LangMap::MAIN_MENU_ABOUT, [](Fl_Widget*, void*)
	{ c::layout::openAboutWindow(); }),
#endif
	    endSubMenu_()};

	copy(popup);

#if G_OS_MAC
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