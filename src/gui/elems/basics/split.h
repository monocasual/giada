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

#ifndef GE_SPLIT_H
#define GE_SPLIT_H

#include "gui/elems/basics/resizerBar.h"
#include <FL/Fl_Group.H>

namespace giada::v
{
/* geSplit
A resizable split-view widget that contains two horizontal panels (A and B). 
TODO - add vertical mode. */

class geSplit : public Fl_Group
{
public:
	enum class Panel
	{
		A,
		B
	};

	geSplit(int x, int y, int w, int h);

	void init(Fl_Widget& a, Fl_Widget& b);
	void resizePanel(Panel p, int s);

private:
	Fl_Widget*   m_a;
	Fl_Widget*   m_b;
	geResizerBar m_bar;
};
} // namespace giada::v

#endif
