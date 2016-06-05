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


#ifndef GD_BROWSER_H
#define GD_BROWSER_H


#include <FL/Fl.H>
#include <FL/Fl_Double_Window.H>
#include "../elems/ge_mixed.h"
#include "../elems/ge_window.h"


class gdBaseBrowser : public gWindow
{
protected:

	class Fl_Group  *groupTop;
	class gBrowser  *browser;
	class gClick    *ok;
	class gClick    *cancel;
	class gInput    *where;
 	class gClick    *updir;
 	class gProgress *status;

	static void cb_up   (Fl_Widget *v, void *p);
	static void cb_down (Fl_Widget *v, void *p);
	static void cb_close(Fl_Widget *w, void *p);

	inline void __cb_up  ();
  inline void __cb_down();
	inline void __cb_close();

	/* Callback
	 * Fired when the save/load button is pressed. */

	void (*callback)(void*);

public:

	gdBaseBrowser(int x, int y, int w, int h, const char *title, const char *path,
			void (*callback)(void*));
	~gdBaseBrowser();

	/* getSelectedItem
	 * Return the full path of the selected file. */

	string getSelectedItem();

	inline gProgress *getStatusBar() { return status; }  // TODO - remove with Patch_DEPR_
	inline void showStatusBar() { status->show(); }
	inline void hideStatusBar() { status->hide(); }
	void setStatusBar(float v);
};


/* -------------------------------------------------------------------------- */


class gdSaveBrowser : public gdBaseBrowser
{
private:

	class gInput *name;

	inline void __cb_down();

public:

	gdSaveBrowser(int x, int y, int w, int h, const char *title, const char *path);
};


/* -------------------------------------------------------------------------- */


class gdLoadBrowser : public gdBaseBrowser
{
private:

	static void cb_load(Fl_Widget *w, void *p);

	inline void __cb_load();
	inline void __cb_down();

public:

	gdLoadBrowser(int x, int y, int w, int h, const char *title, const char *path,
			void (*callback)(void*));
};

#endif
