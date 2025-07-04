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

#ifndef GD_BROWSER_BASE_H
#define GD_BROWSER_BASE_H

#include "src/core/types.h"
#include "src/gui/dialogs/window.h"
#include "src/gui/model.h"
#include <functional>
#include <string>

class Fl_Group;

namespace giada::m
{
class Channel;
}

namespace giada::v
{
class geCheck;
class geImageButton;
class geTextButton;
class geInput;
class geFileBrowser;
class geFlex;
class gdBrowserBase : public gdWindow
{
public:
	~gdBrowserBase();

	/* getSelectedItem
	Returns the full path of the selected file. */

	std::string getSelectedItem() const;

	std::string getCurrentPath() const;
	ID          getChannelId() const;
	void        fireCallback() const;

protected:
	gdBrowserBase(const std::string& title, const std::string& path,
	    std::function<void(void*)> f, ID channelId, const Model&);

	static void cb_toggleHiddenFiles(Fl_Widget* /*w*/, void* p);
	void        cb_toggleHiddenFiles();

	void hidePathName();

	/* m_callback
	Fired when the save/load button is pressed. */

	std::function<void(void*)> m_callback;

	ID m_channelId;

	geCheck*       hiddenFiles;
	geFileBrowser* browser;
	geTextButton*  ok;
	geTextButton*  cancel;
	geFlex*        pathArea;
	geInput*       where;
	geInput*       name;
	geImageButton* updir;
};
} // namespace giada::v

#endif
