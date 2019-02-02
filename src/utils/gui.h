/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#ifndef G_UTILS_GUI_H
#define G_UTILS_GUI_H


#include <string>


class Fl_Window;
class gdWindow;


namespace giada {
namespace u {
namespace gui 
{
/* refresh
Repaints some dynamic GUI elements. */

void refreshUI();

/* shouldBlink
Return whether is time to blink something or not. This is used to make widgets 
blink. */

bool shouldBlink();

/* updateControls
Updates attributes of control elements (sample names, volumes, ...). Useful when 
loading a new patch. */

void updateControls();

/* updateMainWinLabel
Updates the name of the main window */

void updateMainWinLabel(const std::string& s);

void setFavicon(Fl_Window* w);

void openSubWindow(gdWindow* parent, gdWindow* child, int id);

/* refreshActionEditor
Reloads the action editor window by closing and reopening it. It's used when you
delete some actions from the mainWindow and the action editor window is open. */

void refreshActionEditor();

/* closeAllSubwindows
Closes all subwindows attached to mainWin. */

void closeAllSubwindows();

/* getSubwindow
Returns a pointer to an open subwindow, otherwise nullptr. */

gdWindow* getSubwindow(gdWindow* parent, int id);

/* removeFltkChars
Strips special chars used by FLTK to split menus into sub-menus. */

std::string removeFltkChars(const std::string& s);

int getStringWidth(const std::string& s);

}}} // giada::u::gui::


#endif
