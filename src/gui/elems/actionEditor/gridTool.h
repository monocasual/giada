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

#ifndef GE_GRID_TOOL_H
#define GE_GRID_TOOL_H

#include "core/conf.h"
#include "core/types.h"
#include <FL/Fl_Group.H>

class geCheck;

namespace giada::v
{
class geChoice;
class geGridTool : public Fl_Group
{
public:
	geGridTool(Pixel x, Pixel y, m::Conf&);
	~geGridTool();

	int  getValue() const;
	bool isOn() const;

	Frame getSnapFrame(Frame f, Frame framesInBeat) const;

	/* getCellSize
	Returns the size in frames of a single cell of the grid. */

	Frame getCellSize(Frame framesInBeat) const;

private:
	m::Conf& m_conf;

	geChoice* gridType;
	geCheck*  active;
};
} // namespace giada::v

#endif
