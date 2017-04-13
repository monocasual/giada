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


#include "../../../core/const.h"
#include "pianoRoll.h"
#include "pianoItemOrphaned.h"


using namespace giada;


gePianoItemOrphaned::gePianoItemOrphaned(int x, int y, int xRel, int yRel,
  recorder::action *action, gdActionEditor *pParent)
  : Fl_Box  (x, y, WIDTH, gePianoRoll::CELL_H),
    action  (action),
    pParent (pParent),
    selected(false)
{

}


/* -------------------------------------------------------------------------- */


void gePianoItemOrphaned::draw()
{
  fl_rectf(x(), y()+2, WIDTH, h()-3, (Fl_Color) selected ? COLOR_BD_1 : COLOR_BG_2);
}
