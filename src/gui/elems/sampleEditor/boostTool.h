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


#ifndef GE_BOOST_TOOL_H
#define GE_BOOST_TOOL_H


#include <FL/Fl_Group.H>


class geDial;
class geInput;
class geButton;
class geBox;


class geBoostTool : public Fl_Group
{
private:

	giada::m::SampleChannel* ch;

	geBox*    label;
	geDial*   dial;
	geInput*  input;
	geButton* normalize;

	static void cb_setBoost(Fl_Widget* w, void* p);
	static void cb_setBoostNum(Fl_Widget* w, void* p);
	static void cb_normalize(Fl_Widget* w, void* p);
	void cb_setBoost();
	void cb_setBoostNum();
	void cb_normalize();

public:

	geBoostTool(int x, int y, giada::m::SampleChannel* ch);

	void refresh();
};


#endif
