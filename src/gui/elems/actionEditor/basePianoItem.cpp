/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#include "pianoRoll.h"
#include "basePianoItem.h"


geBasePianoItem::geBasePianoItem(int x, int y, int w, gdActionEditor *pParent)
  : Fl_Box  (x, y, w, gePianoRoll::CELL_H),
    pParent (pParent),
    selected(false)
{
}


/* -------------------------------------------------------------------------- */


int geBasePianoItem::getY(int note)
{
  return (gePianoRoll::MAX_KEYS * gePianoRoll::CELL_H) - (note * gePianoRoll::CELL_H);
}
