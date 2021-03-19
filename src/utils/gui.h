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

#ifndef G_UTILS_GUI_H
#define G_UTILS_GUI_H

#include "core/types.h"
#include <string>

namespace giada
{
namespace v
{
class gdWindow;
}
namespace u
{
namespace gui
{
/* refresh
Repaints some dynamic GUI elements. */

void refresh();

/* rebuild
Rebuilds the UI from scratch. Used when the model has changed. */

void rebuild();

/* [rebuild|refresh]SubWindow 
Rebuilds or refreshes subwindow with ID 'wid' if it exists. i.e. if its open. */

void rebuildSubWindow(int wid);
void refreshSubWindow(int wid);

/* shouldBlink
Return whether is time to blink something or not. This is used to make widgets 
blink. */

bool shouldBlink();

/* updateStaticWidgets
Updates attributes of static widgets, i.e. those elements that don't get
automatically refreshed during the UI update loop. Useful when loading a new 
patch. */

void updateStaticWidgets();

/* updateMainWinLabel
Updates the name of the main window */

void updateMainWinLabel(const std::string& s);

void setFavicon(v::gdWindow* w);

void openSubWindow(v::gdWindow* parent, v::gdWindow* child, int id);

// TODO closeSubWindow(...)

/* refreshActionEditor
Reloads the action editor window by closing and reopening it. It's used when you
delete some actions from the mainWindow and the action editor window is open. */

void refreshActionEditor();

/* closeAllSubwindows
Closes all subwindows attached to mainWin. */

void closeAllSubwindows();

/* getSubwindow
Returns a pointer to an open subwindow, otherwise nullptr. */

v::gdWindow* getSubwindow(v::gdWindow* parent, int id);

/* removeFltkChars
Strips special chars used by FLTK to split menus into sub-menus. */

std::string removeFltkChars(const std::string& s);

int getStringWidth(const std::string& s);

/* truncate
Adds ellipsis to a string 's' if it longer than 'width' pixels. */

std::string truncate(const std::string& s, Pixel width);

int centerWindowX(int w);
int centerWindowY(int h);

} // namespace gui
} // namespace u
} // namespace giada

#endif
