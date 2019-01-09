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


#ifndef GE_TAB_PLUGINS_H
#define GE_TAB_PLUGINS_H


#ifdef WITH_VST


#include <FL/Fl_Group.H>


class geInput;
class geButton;
class geBox;


class geTabPlugins : public Fl_Group
{
private:

	geInput* m_folderPath;
	geButton* m_browse;
	geButton* m_scanButton;
	geBox* m_info;

	static void cb_scan(Fl_Widget* w, void* p);
	static void cb_browse(Fl_Widget* w, void* p);
	void cb_scan();
	void cb_browse();

	void refreshCount();

public:

	geTabPlugins(int x, int y, int w, int h);

	void save();
	void refreshVstPath();
};


#endif


#endif
