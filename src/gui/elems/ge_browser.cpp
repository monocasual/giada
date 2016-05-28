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
  printf("loading: %s\n", dir.c_str());
  currentDir = dir;
  load(dir.c_str());
}


/* -------------------------------------------------------------------------- */


string gBrowser::getSelectedItem()
{
  string currentValue = text(value());
  string out;
  if (currentValue[0] == G_SLASH)
    out = currentDir + text(value());
  else
    out = currentDir + (currentDir.back() != G_SLASH ? G_SLASH_STR : "") + text(value());

  printf("currentDir=%s, val=%s ", currentDir.c_str(), text(value()));
  printf("out=%s\n", out.c_str());

  return gGetRealPath(out);
}


/* DEPRECATED STUFF ------------------------------------------------------- */
/* DEPRECATED STUFF ------------------------------------------------------- */
/* DEPRECATED STUFF ------------------------------------------------------- */

/* -------------------------------------------------------------------------- */


void gBrowser::__DEPR__init(const char *init_path) {

	gLog("[gBrowser] init path = '%s'\n", init_path);

	if (init_path == NULL || !gIsDir(init_path)) {

#if defined(__linux__) || defined(__APPLE__)

		__DEPR__path->value("/home");

#elif defined(_WIN32)

		/* SHGetFolderPath is deprecated. We should use SHGetKnownFolderPath
		 * but that would break compatibility with XP. On Vista, GetFolderPath
		 * is a wrapper of GetKnownFolderPath, so no problem. */

		char winRoot[1024];
		SHGetFolderPath(NULL, CSIDL_COMMON_DESKTOPDIRECTORY, NULL, 0, winRoot); // si parte dal Desktop
		path->value(winRoot);

#endif

		gLog("[gBrowser] init_path null or invalid, using default\n");
	}
	else
		__DEPR__path->value(init_path);

	__DEPR__refresh();
	sort();
}


/* -------------------------------------------------------------------------- */


void gBrowser::__DEPR__refresh() {
  DIR *dp;
  struct dirent *ep;
  dp = opendir(__DEPR__path->value());
  if (dp != NULL) {
		while ((ep = readdir(dp))) {

			/* skip:
			 * - "." e ".."
			 * - hidden files */

			if (strcmp(ep->d_name, ".") != 0 && strcmp(ep->d_name, "..") != 0) {
				if (ep->d_name[0] != '.') {

					/* is it a folder? add square brackets. Is it a file? Append
					 * a '/' (on Windows seems useless, though) */

					std::string file = __DEPR__path->value();
					file.insert(file.size(), gGetSlash());
					file += ep->d_name;

					if (gIsDir(file.c_str())) {
						char name[PATH_MAX];
						sprintf(name, "@b[%s]", ep->d_name);
						add(name);
					}
					else
					if (gIsProject(file.c_str())) {
						char name[PATH_MAX];
						sprintf(name, "@i@b%s", ep->d_name);
						add(name);
					}
					else
						add(ep->d_name);
				}
			}
		}
		closedir(dp);
  }
  else
    gLog("[gBrowser] Couldn't open the directory '%s'\n", __DEPR__path->value());
}


/* -------------------------------------------------------------------------- */


void gBrowser::__DEPR__up_dir() {

	/* updir = remove last folder from the path. Start from strlen(-1) to
	 * skip the trailing slash */

	int i = strlen(__DEPR__path->value())-1;

	/* on Windows an updir from the path "X:\" (3 chars long) must redirect
	 * to the list of available devices. */

#if defined(_WIN32)
	if (i <= 3 || !strcmp(__DEPR__path->value(), "All drives")) {
		path->value("All drives");
		showDrives();
		return;
	}
	else {
		while (i >= 0) {
			if (__DEPR__path->value()[i] == '\\')
				break;
			i--;
		}

		/* delete the last part of the string, from i to len-i, ie everything
		 * after the "/" */

		std::string tmp = __DEPR__path->value();
		tmp.erase(i, tmp.size()-i);

		/* if tmp.size == 2 we have something like 'C:'. Add a trailing
		 * slash */

		if (tmp.size() == 2)
			tmp += "\\";

		__DEPR__path->value(tmp.c_str());
		refresh();
	}
#elif defined(__linux__) || defined (__APPLE__)
	while (i >= 0) {
		if (__DEPR__path->value()[i] == '/')
			break;
		i--;
	}

	/* i == 0 means '/', the root dir. It's meaningless to go updir */

	if (i==0)
		__DEPR__path->value("/");
	else {

		/* delete the last part of the string, from i to len-i, ie everything
		 * after the "/" */

		std::string tmp = __DEPR__path->value();
		tmp.erase(i, tmp.size()-i);
		__DEPR__path->value(tmp.c_str());
	}
	__DEPR__refresh();
#endif
}


/* -------------------------------------------------------------------------- */


void gBrowser::__DEPR__down_dir(const char *p) {
	__DEPR__path->value(p);
	__DEPR__refresh();
}


/* -------------------------------------------------------------------------- */


const char *gBrowser::__DEPR__get_selected_item() {

	/* click on an empty line */

	if (text(value()) == NULL)
		return NULL;

	__DEPR__selected_item = text(value());

	/* @ = formatting marks.
	 * @b = bold, i.e. a directory. Erease '@b[' and ']' */

	if (__DEPR__selected_item[0] == '@') {
		if (__DEPR__selected_item[1] == 'b') {
			__DEPR__selected_item.erase(0, 3);
			__DEPR__selected_item.erase(__DEPR__selected_item.size()-1, 1);
		}
		else
		if (__DEPR__selected_item[1] == 'i')
			__DEPR__selected_item.erase(0, 4);
	}

#if defined(__linux__) || defined(__APPLE__)

	/* add path to file name, to get an absolute path. Avoid double
	 * slashes like '//' */

	if (strcmp("/", __DEPR__path->value()))
		__DEPR__selected_item.insert(0, "/");

	__DEPR__selected_item.insert(0, __DEPR__path->value());
	return __DEPR__selected_item.c_str();
#elif defined(_WIN32)

	/* if path is 'All drives' we are in the devices list and the user
	 * has clicked on a device such as 'X:\' */

	if (strcmp(__DEPR__path->value(), "All drives") == 0)
			return __DEPR__selected_item.c_str();
	else {

		/* add '\' if the path is like 'X:\' */

		if (strlen(__DEPR__path->value()) > 3) /// shouln't it be == 3?
			__DEPR__selected_item.insert(0, "\\");

		__DEPR__selected_item.insert(0, path->value());
		return __DEPR__selected_item.c_str();
	}
#endif
}


/* -------------------------------------------------------------------------- */


#ifdef _WIN32
void gBrowser::showDrives() {

	/* GetLogicalDriveStrings fills drives like that:
	 *
	 * a:\[null]b:\[null]c:\[null]...[null][null]
	 *
	 * where [null] stands for \0. */

	char drives[64];
	char *i = drives;		// pointer to 0th element in drives
	GetLogicalDriveStrings(64, drives);

	/* code stolen from the web, still unknown. (Jan 09, 2012). */

	while (*i) {
		add(i);
		i = &i[strlen(i) + 1];
	}
}

#endif
