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


#ifndef GD_BROWSER_BASE_H
#define GD_BROWSER_BASE_H


#include "../window.h"


class Fl_Group;
class Channel;
class geCheck;
class geBrowser;
class geButton;
class geInput;
class geProgress;


class gdBrowserBase : public gdWindow
{
protected:

	Channel* channel;

	Fl_Group* groupTop;
	geCheck* hiddenFiles;
	geBrowser* browser;
	geButton* ok;
	geButton* cancel;
	geInput* where;
	geButton* updir;
	geProgress* status;

	static void cb_up(Fl_Widget* v, void* p);
	static void cb_close(Fl_Widget* w, void* p);
	static void cb_toggleHiddenFiles(Fl_Widget* w, void* p);
	void cb_up();
	void cb_close();
	void cb_toggleHiddenFiles();

	/* Callback
	 * Fired when the save/load button is pressed. */

	void (*callback)(void*);

	gdBrowserBase(int x, int y, int w, int h, const std::string& title,
			const std::string& path, void (*callback)(void*));

public:

	~gdBrowserBase();

	/* getSelectedItem
	 * Return the full path of the selected file. */

	std::string getSelectedItem() const;

	std::string getCurrentPath() const;
	Channel* getChannel() const;
	void fireCallback() const;
	
	/* setStatusBar
	 * Increment status bar for progress tracking. */

	void setStatusBar(float v);

	void showStatusBar();
	void hideStatusBar();

};


#endif
