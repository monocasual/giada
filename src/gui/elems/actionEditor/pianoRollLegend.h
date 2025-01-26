/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef GE_PIANO_ROLL_LEGEND_H
#define GE_PIANO_ROLL_LEGEND_H

#include "core/types.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/scroll.h"
#include <FL/fl_draw.H>

namespace giada::v
{
class gePianoRollLegend : public geScroll
{
public:
	gePianoRollLegend();

	void draw() override;

private:
	class geContent : public geBox
	{
	public:
		geContent(Pixel x, Pixel y, Pixel w);

		void draw() override;

	private:
		/* drawOffscreen
		Generates a complex drawing in memory first and copy it to the screen at
		a later point in time. Fl_Offscreen surface holds the necessary data. */

		void drawOffscreen();

		Fl_Offscreen m_offscreen; // For drawing vertical notes
	};
};
} // namespace giada::v

#endif
