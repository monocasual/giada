/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * geScroll
 * Custom scroll with nice scrollbars and something else.
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef GE_GROUP_H
#define GE_GROUP_H

#include <FL/Fl_Group.H>
#include <cstddef>
#include <vector>

namespace giada
{
namespace v
{
/* geGroup
A group that resizes itself according to the content. */

class geGroup : public Fl_Group
{
public:
	geGroup(int x, int y);

	/* countChildren
	Returns the number of widgets contained in this group. */

	std::size_t countChildren() const;

	/* add
	Adds a Fl_Widget 'w' to this group. Coordinates are relative to the group,
	so origin starts at (0, 0). As with any other FLTK group, the widget becomes
	owned by this group: If you add static or automatic (local) variables, then
	it is your responsibility to remove (or delete) all such static or automatic
	child widgets before destroying the group - otherwise the child widgets'
	destructors would be called twice! */

	void add(Fl_Widget* w);

	Fl_Widget* getChild(std::size_t i);
	Fl_Widget* getLastChild();

private:
	/* m_widgets
	The internal Fl_Scroll::array_ is unreliable when inspected with the child()
	method. Let's keep track of widgets that belong to this group manually. */

	std::vector<Fl_Widget*> m_widgets;
};
} // namespace v
} // namespace giada

#endif
