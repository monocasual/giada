/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * boxtypes
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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


#ifndef GE_BOXTYPES_H
#define GE_BOXTYPES_H


#include <FL/Fl.H>


#define G_CUSTOM_BORDER_BOX FL_FREE_BOXTYPE
#define G_CUSTOM_UP_BOX     (Fl_Boxtype)(FL_FREE_BOXTYPE + 1)
#define G_CUSTOM_DOWN_BOX   (Fl_Boxtype)(FL_FREE_BOXTYPE + 3)


void g_customBorderBox(int x, int y, int w, int h, Fl_Color c);
void g_customUpBox    (int x, int y, int w, int h, Fl_Color c);
void g_customDownBox  (int x, int y, int w, int h, Fl_Color c);


#endif
