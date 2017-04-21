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


#ifndef GD_BROWSER_H
#define GD_BROWSER_H


#include "window.h"


class Channel;
class Fl_Group;
class gCheck;
class geBrowser;
class geButton;
class geInput;
class gProgress;


class gdBaseBrowser : public gdWindow
{
protected:

	Fl_Group  *groupTop;
  gCheck    *hiddenFiles;
	geBrowser *browser;
	geButton  *ok;
	geButton  *cancel;
	geInput   *where;
 	geButton  *updir;
 	gProgress *status;

	static void cb_up               (Fl_Widget *v, void *p);
	static void cb_close            (Fl_Widget *w, void *p);
	static void cb_toggleHiddenFiles(Fl_Widget *w, void *p);

	inline void __cb_up               ();
	inline void __cb_close            ();
	inline void __cb_toggleHiddenFiles();

	/* Callback
	 * Fired when the save/load button is pressed. */

	void (*callback)(void*);

	Channel *channel;

public:

	gdBaseBrowser(int x, int y, int w, int h, const std::string &title,
			const std::string &path,	void (*callback)(void*));

	~gdBaseBrowser();

	/* getSelectedItem
	 * Return the full path of the selected file. */

	std::string getSelectedItem();

	/* setStatusBar
	 * Increment status bar for progress tracking. */

	void setStatusBar(float v);

	gProgress *getStatusBar() { return status; }  // TODO - remove with Patch_DEPR_
	void showStatusBar();
	void hideStatusBar();
  std::string getCurrentPath();

	Channel *getChannel() { return channel; }
	void fireCallback()   { callback((void*) this); }
};


/* -------------------------------------------------------------------------- */


class gdSaveBrowser : public gdBaseBrowser
{
private:

	geInput *name;

	static void cb_down(Fl_Widget *v, void *p);
	static void cb_save(Fl_Widget *w, void *p);

	inline void __cb_down();
	inline void __cb_save();

public:

	gdSaveBrowser(int x, int y, int w, int h, const std::string &title,
			const std::string &path,	const std::string &name, void (*callback)(void*),
			Channel *ch);

	std::string getName();
};


/* -------------------------------------------------------------------------- */


class gdLoadBrowser : public gdBaseBrowser
{
private:

	static void cb_load(Fl_Widget *w, void *p);
	static void cb_down(Fl_Widget *v, void *p);

	inline void __cb_load();
	inline void __cb_down();

public:

	gdLoadBrowser(int x, int y, int w, int h, const std::string &title,
			const std::string &path,	void (*callback)(void*), Channel *ch);
};

#endif
