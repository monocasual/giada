/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#ifndef GE_PIANO_ROLL_H
#define GE_PIANO_ROLL_H

#include "baseActionEditor.h"
#include <FL/fl_draw.H>

namespace giada::m
{
class MidiChannel;
class Action;
} // namespace giada::m
namespace giada::v
{
class gePianoRoll : public geBaseActionEditor
{
public:
	static const int   MAX_KEYS    = 127;
	static const int   MAX_OCTAVES = 9;
	static const int   KEYS        = 12;
	static const Pixel CELL_H      = 20;
	static const Pixel CELL_W      = 40;

	gePianoRoll(Pixel x, Pixel y, Pixel w, gdBaseActionEditor*);

	void draw() override;
	int  handle(int e) override;

	void rebuild(c::actionEditor::Data& d) override;

	Pixel pick;

  private:
	enum class Notes
	{
		G  = 1,
		FS = 2,
		F  = 3,
		E  = 4,
		DS = 5,
		D  = 6,
		CS = 7,
		C  = 8,
		B  = 9,
		AS = 10,
		A  = 11,
		GS = 0
	};

	Fl_Offscreen surface1; // notes, no repeat
	Fl_Offscreen surface2; // lines, x-repeat

	void onAddAction() override;
	void onDeleteAction() override;
	void onMoveAction() override;
	void onResizeAction() override;
	void onRefreshAction() override;

	/* drawSurface*
	Generates a complex drawing in memory first and copy it to the screen at a
	later point in time. Fl_Offscreen surface holds the necessary data.	The first
	call creates an offscreen surface of CELL_W pixel wide containing note values.
	The second call creates another offscreen surface of CELL_W pixels wide
	containing the rest of the piano roll. The latter will then be tiled during
	the ::draw() call. */

	void drawSurface1();
	void drawSurface2();

	Pixel snapToY(Pixel p) const;
	int   yToNote(Pixel y) const;
	Pixel noteToY(int n) const;
	Pixel getPianoItemW(Pixel x, const m::Action& a1, const m::Action& a2) const;
};
} // namespace giada::v

#endif
