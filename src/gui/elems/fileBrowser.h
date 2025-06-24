/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_browser
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

#ifndef GE_FILE_BROWSER_H
#define GE_FILE_BROWSER_H

#include <FL/Fl_File_Browser.H>
#include <functional>
#include <string>

namespace giada::v
{
class geFileBrowser : public Fl_File_Browser
{
public:
	geFileBrowser();

	int handle(int e) override;

	/* init
	Initializes browser and show 'dir' as initial directory. */

	void loadDir(const std::string& dir);

	/* getSelectedItem
	Returns the full path or of the selected item. */

	std::string getSelectedItem() const;

	std::string getCurrentDir() const;
	void        toggleHiddenFiles();
	void        preselect(int position, int line);

	/* onChooseItem
	Callback fired when an element has been selected by pressing Enter or
	double-clicking on it. */

	std::function<void()> onChooseItem;

private:
	std::string m_currentDir;
	bool        m_showHiddenFiles;
};
} // namespace giada::v

#endif
