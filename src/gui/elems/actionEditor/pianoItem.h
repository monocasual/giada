/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
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

#ifndef GE_PIANO_ITEM_H
#define GE_PIANO_ITEM_H

#include "baseAction.h"

namespace giada::m
{
struct Action;
}

namespace giada::v
{
class gePianoItem : public geBaseAction
{
public:
	gePianoItem(int x, int y, int w, int h, m::Action a1, m::Action a2);

	void draw() override;

	bool isResizable() const;

private:
	bool m_ringLoop;
	bool m_orphaned;

	Pixel calcVelocityH() const;
};
} // namespace giada::v

#endif
