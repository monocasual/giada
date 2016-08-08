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
#include "../dialogs/gd_browser.h"
#include "ge_browser.h"


gBrowser::gBrowser(int x, int y, int w, int h)
 : Fl_File_Browser(x, y, w, h)
{
	box(G_BOX);
	textsize(GUI_FONT_SIZE_BASE);
	textcolor(COLOR_TEXT_0);
	selection_color(COLOR_BG_1);
	color(COLOR_BG_0);
  type(FL_SELECT_BROWSER);

	this->scrollbar.color(COLOR_BG_0);
	this->scrollbar.selection_color(COLOR_BG_1);
	this->scrollbar.labelcolor(COLOR_BD_1);
	this->scrollbar.slider(G_BOX);

	this->hscrollbar.color(COLOR_BG_0);
	this->hscrollbar.selection_color(COLOR_BG_1);
	this->hscrollbar.labelcolor(COLOR_BD_1);
	this->hscrollbar.slider(G_BOX);

  take_focus();  // let it have focus on startup
}


/* -------------------------------------------------------------------------- */


void gBrowser::loadDir(const string &dir)
{
  currentDir = dir;
  load(currentDir.c_str());

  /* hide "../", it just screws up things  */

  if (text(1) != NULL && strcmp(text(1), "../") == 0)
    remove(1);
}


/* -------------------------------------------------------------------------- */

int gBrowser::handle(int e)
{
	int ret = Fl_File_Browser::handle(e);
  switch (e) {
    case FL_FOCUS:
		case FL_UNFOCUS:
			ret = 1;                	// enables receiving Keyboard events
			break;
    case FL_KEYDOWN:  // keyboard
      if (Fl::event_key(FL_Down))
        select(value() + 1);
      else
      if (Fl::event_key(FL_Up))
        select(value() - 1);
      else
      if (Fl::event_key(FL_Enter))
        ((gdBaseBrowser*) parent())->fireCallback();
      ret = 1;
      break;
    case FL_PUSH:    // mouse
      if (Fl::event_clicks() > 0)  // double click
        ((gdBaseBrowser*) parent())->fireCallback();
      ret = 1;
      break;
    case FL_RELEASE: // mouse
      /* nasty trick to keep the selection on mouse release */
      if (value() > 1) {
        select(value() - 1);
        select(value() + 1);
      }
      else {
        select(value() + 1);
        select(value() - 1);
      }
      ret = 1;
      break;
  }
	return ret;
}

/* -------------------------------------------------------------------------- */


string gBrowser::getCurrentDir()
{
  return normalize(gGetRealPath(currentDir));
}


/* -------------------------------------------------------------------------- */


string gBrowser::getSelectedItem(bool fullPath)
{
  if (!fullPath)     // no full path requested? return the selected text
    return normalize(text(value()));
  else
  if (value() == 0)  // no rows selected? return current directory
    return normalize(currentDir);
  else
    return normalize(gGetRealPath(currentDir + text(value())));
}


/* -------------------------------------------------------------------------- */


void gBrowser::preselect(int pos, int line)
{
  position(pos);
  select(line);
}


/* -------------------------------------------------------------------------- */


string gBrowser::normalize(const string &s)
{
  string out = s;

  /* our crappy version of Clang doesn't seem to support std::string::back() */

#ifdef __APPLE__
  if (out[out.length() - 1] == G_SLASH)
#else
  if (out.back() == G_SLASH)
#endif

    out = out.substr(0, out.size()-1);
  return out;
}
