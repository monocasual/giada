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


#include "core/const.h"
#include "utils/string.h"
#include "utils/fs.h"
#include "gui/dialogs/browser/browserBase.h"
#include "basics/boxtypes.h"
#include "browser.h"


namespace giada {
namespace v
{
geBrowser::geBrowser(int x, int y, int w, int h)
: Fl_File_Browser  (x, y, w, h),
  m_showHiddenFiles(false)
{
	box(G_CUSTOM_BORDER_BOX);
	textsize(G_GUI_FONT_SIZE_BASE);
	textcolor(G_COLOR_LIGHT_2);
	selection_color(G_COLOR_GREY_4);
	color(G_COLOR_GREY_2);
	type(FL_SELECT_BROWSER);

	this->scrollbar.color(G_COLOR_GREY_2);
	this->scrollbar.selection_color(G_COLOR_GREY_4);
	this->scrollbar.labelcolor(G_COLOR_LIGHT_1);
	this->scrollbar.slider(G_CUSTOM_BORDER_BOX);

	this->hscrollbar.color(G_COLOR_GREY_2);
	this->hscrollbar.selection_color(G_COLOR_GREY_4);
	this->hscrollbar.labelcolor(G_COLOR_LIGHT_1);
	this->hscrollbar.slider(G_CUSTOM_BORDER_BOX);

	take_focus();  // let it have focus on startup
}


/* -------------------------------------------------------------------------- */


void geBrowser::toggleHiddenFiles()
{
	m_showHiddenFiles = !m_showHiddenFiles;
	loadDir(m_currentDir);
}


/* -------------------------------------------------------------------------- */


void geBrowser::loadDir(const std::string& dir)
{
	m_currentDir = dir;
	load(m_currentDir.c_str());

	/* Clean up unwanted elements. Hide "../" first, it just screws up things.
	Also remove hidden files, if requested. */

	for (int i=size(); i>=0; i--) {
		if (text(i) == nullptr)
			continue;
		if (strcmp(text(i), "../") == 0 || (!m_showHiddenFiles && strncmp(text(i), ".", 1) == 0))
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
				static_cast<v::gdBrowserBase*>(parent())->fireCallback();
			ret = 1;
			break;
		case FL_PUSH:    // mouse
			if (Fl::event_clicks() > 0)  // double click
				static_cast<v::gdBrowserBase*>(parent())->fireCallback();
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


std::string geBrowser::getCurrentDir()
{
	return normalize(u::string::getRealPath(m_currentDir));
}


/* -------------------------------------------------------------------------- */


std::string geBrowser::getSelectedItem(bool fullPath)
{
	if (!fullPath)     // no full path requested? return the selected text
		return normalize(text(value()));
	else
	if (value() == 0)  // no rows selected? return current directory
		return normalize(m_currentDir);
	else {
#ifdef G_OS_WINDOWS
		std::string sep = m_currentDir != "" ? G_SLASH_STR : "";
#else
		std::string sep = G_SLASH_STR;
#endif
		return normalize(u::string::getRealPath(m_currentDir + sep + normalize(text(value()))));
	}
}


/* -------------------------------------------------------------------------- */


void geBrowser::preselect(int pos, int line)
{
	position(pos);
	select(line);
}


/* -------------------------------------------------------------------------- */


std::string geBrowser::normalize(const std::string& s)
{
	std::string out = s;

	/* If std::string ends with G_SLASH, remove it. Don't do it if is the root dir, 
	that is '/' on Unix or '[x]:\' on Windows. */

	//if (out.back() == G_SLASH && out.length() > 1)
	if (out.back() == G_SLASH && !gu_isRootDir(s))
		out = out.substr(0, out.size()-1);
	return out;
}
}} // giada::v::