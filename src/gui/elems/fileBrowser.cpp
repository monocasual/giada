/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "fileBrowser.h"
#include "basics/boxtypes.h"
#include "core/const.h"
#include "gui/dialogs/browser/browserBase.h"
#include "utils/fs.h"
#include "utils/string.h"

namespace giada::v
{
geFileBrowser::geFileBrowser()
: Fl_File_Browser(0, 0, 0, 0)
, onSelectedElement(nullptr)
, m_showHiddenFiles(false)
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

	take_focus(); // let it have focus on startup
}

/* -------------------------------------------------------------------------- */

void geFileBrowser::toggleHiddenFiles()
{
	m_showHiddenFiles = !m_showHiddenFiles;
	loadDir(m_currentDir);
}

/* -------------------------------------------------------------------------- */

void geFileBrowser::loadDir(const std::string& dir)
{
	m_currentDir = dir;
	load(m_currentDir.c_str());

	/* Clean up unwanted elements. Hide "../" first, it just screws up things.
	Also remove hidden files, if requested. */

	for (int i = size(); i >= 0; i--)
	{
		if (text(i) == nullptr)
			continue;
		const std::string_view el = text(i);
		if (el == "../" || (!m_showHiddenFiles && el[0] == '.'))
			remove(i);
	}
}

/* -------------------------------------------------------------------------- */

int geFileBrowser::handle(int e)
{
	int ret = Fl_File_Browser::handle(e);
	switch (e)
	{
	case FL_FOCUS:
	case FL_UNFOCUS:
		ret = 1; // enables receiving Keyboard events
		break;
	case FL_KEYDOWN: // keyboard
		if (Fl::event_key(FL_Down))
			select(value() + 1);
		else if (Fl::event_key(FL_Up))
			select(value() - 1);
		else if (Fl::event_key(FL_Enter) && onSelectedElement != nullptr)
			onSelectedElement();
		ret = 1;
		break;
	case FL_PUSH:                                                   // mouse
		if (Fl::event_clicks() > 0 && onSelectedElement != nullptr) // double click
			onSelectedElement();
		ret = 1;
		break;
	case FL_RELEASE: // mouse
		/* nasty trick to keep the selection on mouse release */
		if (value() > 1)
		{
			select(value() - 1);
			select(value() + 1);
		}
		else
		{
			select(value() + 1);
			select(value() - 1);
		}
		ret = 1;
		break;
	}
	return ret;
}

/* -------------------------------------------------------------------------- */

std::string geFileBrowser::getCurrentDir()
{
	return u::fs::getRealPath(m_currentDir);
}

/* -------------------------------------------------------------------------- */

std::string geFileBrowser::getSelectedItem(bool fullPath)
{
	if (value() == 0) // no rows selected? return current directory
		return m_currentDir;
	if (!fullPath) // no full path requested? return the selected text
		return text(value());
	else
		return u::fs::getRealPath(u::fs::join(m_currentDir, text(value())));
}

/* -------------------------------------------------------------------------- */

void geFileBrowser::preselect(int pos, int line)
{
	vposition(pos);
	select(line);
}
} // namespace giada::v