/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#include "../../core/const.h"
#include "../../utils/string.h"
#include "../dialogs/gd_browser.h"
#include "basics/boxtypes.h"
#include "browser.h"


using std::string;


geBrowser::geBrowser(int x, int y, int w, int h)
 : Fl_File_Browser(x, y, w, h),
   showHiddenFiles(false)
{
	box(G_CUSTOM_BORDER_BOX);
	textsize(GUI_FONT_SIZE_BASE);
	textcolor(COLOR_TEXT_0);
	selection_color(COLOR_BG_1);
	color(COLOR_BG_0);
  type(FL_SELECT_BROWSER);

	this->scrollbar.color(COLOR_BG_0);
	this->scrollbar.selection_color(COLOR_BG_1);
	this->scrollbar.labelcolor(COLOR_BD_1);
	this->scrollbar.slider(G_CUSTOM_BORDER_BOX);

	this->hscrollbar.color(COLOR_BG_0);
	this->hscrollbar.selection_color(COLOR_BG_1);
	this->hscrollbar.labelcolor(COLOR_BD_1);
	this->hscrollbar.slider(G_CUSTOM_BORDER_BOX);

  take_focus();  // let it have focus on startup
}


/* -------------------------------------------------------------------------- */


void geBrowser::toggleHiddenFiles()
{
  showHiddenFiles = !showHiddenFiles;
  loadDir(currentDir);
}


/* -------------------------------------------------------------------------- */


void geBrowser::loadDir(const string &dir)
{
  currentDir = dir;
  load(currentDir.c_str());

  /* Clean up unwanted elements. Hide "../" first, it just screws up things.
  Also remove hidden files, if requested. */

  for (int i=size(); i>=0; i--) {
    if (text(i) == nullptr)
      continue;
    if (strcmp(text(i), "../") == 0 || (!showHiddenFiles && strncmp(text(i), ".", 1) == 0))
      remove(i);
  }
}


/* -------------------------------------------------------------------------- */

int geBrowser::handle(int e)
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


string geBrowser::getCurrentDir()
{
  return normalize(gu_getRealPath(currentDir));
}


/* -------------------------------------------------------------------------- */


string geBrowser::getSelectedItem(bool fullPath)
{
  if (!fullPath)     // no full path requested? return the selected text
    return normalize(text(value()));
  else
  if (value() == 0)  // no rows selected? return current directory
    return normalize(currentDir);
  else
    return normalize(gu_getRealPath(currentDir + G_SLASH + normalize(text(value()))));
}


/* -------------------------------------------------------------------------- */


void geBrowser::preselect(int pos, int line)
{
  position(pos);
  select(line);
}


/* -------------------------------------------------------------------------- */


string geBrowser::normalize(const string &s)
{
  string out = s;

  /* If string ends with G_SLASH, remove it. Don't do it if has length > 1, it
  means that the string is just '/'. Note: our crappy version of Clang doesn't
  seem to support std::string::back() */

#ifdef __APPLE__
  if (out[out.length() - 1] == G_SLASH && out.length() > 1)
#else
  if (out.back() == G_SLASH && out.length() > 1)
#endif

    out = out.substr(0, out.size()-1);
  return out;
}
