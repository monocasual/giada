/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_browser
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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


#include <limits.h>
#include "../../core/const.h"
#include "../../utils/utils.h"
#include "../../utils/string.h"
#include "../../utils/log.h"
#include "ge_browser.h"


gBrowser::gBrowser(int x, int y, int w, int h)
 : Fl_File_Browser(x, y, w, h)
{
	box(G_BOX);
	textsize(GUI_FONT_SIZE_BASE);
	textcolor(COLOR_TEXT_0);
	selection_color(COLOR_BG_1);
	color(COLOR_BG_0);
  type(FL_HOLD_BROWSER); // single selection

	this->scrollbar.color(COLOR_BG_0);
	this->scrollbar.selection_color(COLOR_BG_1);
	this->scrollbar.labelcolor(COLOR_BD_1);
	this->scrollbar.slider(G_BOX);

	this->hscrollbar.color(COLOR_BG_0);
	this->hscrollbar.selection_color(COLOR_BG_1);
	this->hscrollbar.labelcolor(COLOR_BD_1);
	this->hscrollbar.slider(G_BOX);

}


/* -------------------------------------------------------------------------- */


void gBrowser::loadDir(const string &dir)
{
  currentDir = dir;
  if (currentDir.back() != G_SLASH)  // make sure the trailing slash exists
    currentDir += G_SLASH;
  load(currentDir.c_str());
}


/* -------------------------------------------------------------------------- */


string gBrowser::getCurrentDir()
{
  return gGetRealPath(currentDir);
}


/* -------------------------------------------------------------------------- */


string gBrowser::getSelectedItem(bool fullPath)
{
  if (!fullPath)     // no full path requested? return the selected text
    return text(value());

  if (value() == 0)  // no rows selected? return current directory
    return currentDir;

  return gGetRealPath(currentDir + text(value()));
}
