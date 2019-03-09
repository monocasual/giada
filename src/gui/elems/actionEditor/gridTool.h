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


#ifndef GE_GRID_TOOL_H
#define GE_GRID_TOOL_H


#include <FL/Fl_Group.H>
#include "core/types.h"


class geChoice;
class geCheck;


namespace giada {
namespace v
{
class geGridTool : public Fl_Group
{
private:

  geChoice* gridType;
	geCheck*  active;

	static void cb_changeType(Fl_Widget* w, void* p);
	inline void cb_changeType();

public:

	geGridTool(Pixel x, Pixel y);
	~geGridTool();

	int getValue() const;
	bool isOn() const;

	Frame getSnapFrame(Frame f) const;

	/* getCellSize
	Returns the size in frames of a single cell of the grid. */

	Frame getCellSize() const;

};
}} // giada::v::


#endif
