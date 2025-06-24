/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "gui/elems/fileBrowser.h"
#include "basics/boxtypes.h"
#include "core/const.h"
#include "utils/fs.h"

namespace giada::v
{
geFileBrowser::geFileBrowser()
: Fl_File_Browser(0, 0, 0, 0)
, onChooseItem(nullptr)
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

	take_focus();        // let it have focus on startup
	when(FL_WHEN_NEVER); // Disable callback from this widget
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
	switch (e)
	{
	case FL_FOCUS:
	case FL_UNFOCUS:
		return 1;    // enables receiving Keyboard events
	case FL_KEYDOWN: // keyboard
		if (Fl::event_key(FL_Down))
			select(value() + 1);
		else if (Fl::event_key(FL_Up))
			select(value() - 1);
		else if (Fl::event_key(FL_Enter))
			if (onChooseItem != nullptr)
				onChooseItem();
		return 1;
	case FL_PUSH:                   // mouse
		if (Fl::event_clicks() > 0) // double click
		{
			const int ret = Fl_File_Browser::handle(e);
			/* Do stuff only if you double-clicked a file name and nothing else
			e.g. the scrollbars, otherwise it screws the default behavior when
			double-clicking on the scrollbar arrows. */
			if (onChooseItem != nullptr && Fl::belowmouse() == this)
				onChooseItem();
			return ret;
		}
		else
			return Fl_File_Browser::handle(e);
	case FL_RELEASE: // mouse
		return 1;    // Prevents losing selection on mouse up
	default:
		return Fl_File_Browser::handle(e);
	}
}

/* -------------------------------------------------------------------------- */

std::string geFileBrowser::getCurrentDir() const
{
	return u::fs::getRealPath(m_currentDir);
}

/* -------------------------------------------------------------------------- */

std::string geFileBrowser::getSelectedItem() const
{
	if (value() == 0) // no rows selected? return current directory
		return m_currentDir;
	return u::fs::getRealPath(u::fs::join(m_currentDir, text(value())));
}

/* -------------------------------------------------------------------------- */

void geFileBrowser::preselect(int pos, int line)
{
	vposition(pos);
	select(line);
}
} // namespace giada::v