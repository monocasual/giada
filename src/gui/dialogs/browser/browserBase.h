/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#include "core/types.h"
#include "gui/dialogs/window.h"
#include <functional>
#include <string>

class Fl_Group;
class geCheck;
class geButton;
class geInput;
class geProgress;

namespace giada
{
namespace m
{
class Channel;
}
namespace v
{
class geBrowser;
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

	/* setStatusBar
	Increments status bar for progress tracking. */

	void setStatusBar(float v);

	void showStatusBar();
	void hideStatusBar();

  protected:
	gdBrowserBase(const std::string& title, const std::string& path,
	    std::function<void(void*)> f, ID channelId);

	static void cb_up(Fl_Widget* /*w*/, void* p);
	static void cb_close(Fl_Widget* /*w*/, void* p);
	static void cb_toggleHiddenFiles(Fl_Widget* /*w*/, void* p);
	void        cb_up();
	void        cb_close();
	void        cb_toggleHiddenFiles();

	/* m_callback
	Fired when the save/load button is pressed. */

	std::function<void(void*)> m_callback;

	ID m_channelId;

	Fl_Group*   groupTop;
	geCheck*    hiddenFiles;
	geBrowser*  browser;
	geButton*   ok;
	geButton*   cancel;
	geInput*    where;
	geButton*   updir;
	geProgress* status;
};
} // namespace v
} // namespace giada

#endif
