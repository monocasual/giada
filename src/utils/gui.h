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


/* refresh
 * refresh all GUI elements. */

void gu_refreshUI();

/* getBlinker
*  return blinker value, used to make widgets blink. */

int gu_getBlinker();

/* updateControls
 * update attributes of control elements (sample names, volumes, ...).
 * Useful when loading a new patch. */

void gu_updateControls();

/* update_win_label
 * update the name of the main window */

void gu_updateMainWinLabel(const std::string &s);

void gu_setFavicon(Fl_Window *w);

void gu_openSubWindow(gdWindow *parent, gdWindow *child, int id);

/* refreshActionEditor
 * reload the action editor window by closing and reopening it. It's used
 * when you delete some actions from the mainWindow and the action editor
 * window is open. */

void gu_refreshActionEditor();

/* closeAllSubwindows
 * close all subwindows attached to mainWin. */

void gu_closeAllSubwindows();

/* getSubwindow
 * return a pointer to an open subwindow, otherwise nullptr. */

gdWindow *gu_getSubwindow(gdWindow *parent, int id);

/* removeFltkChars
 * Strip special chars used by FLTK to split menus into sub-menus. */

std::string gu_removeFltkChars(const std::string &s);

int gu_getStringWidth(const std::string &s);


#endif
