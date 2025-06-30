/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * beatMeter
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

#ifndef GE_SEQUENCER_H
#define GE_SEQUENCER_H

#include "deps/geompp/src/rect.hpp"
#include "glue/main.h"
#include "gui/elems/basics/box.h"

namespace giada::v
{
class geSequencer : public geBox
{
public:
	geSequencer();

	void draw() override;

	void refresh();

private:
	static constexpr int REC_BARS_H = 3;
	static constexpr int CURSOR_PAD = 3;

	void drawBody() const;
	void drawCursor() const;
	void drawCursor(int beat) const;
	void drawRecBars() const;
	void drawBeatNumber(int beat, Fl_Color) const;

	c::main::Sequencer m_data;

	geompp::Rect<int> m_background;
	geompp::Rect<int> m_cell;
};
} // namespace giada::v

#endif
