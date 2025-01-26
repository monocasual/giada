/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * boxtypes
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

#ifndef GE_BOXTYPES_H
#define GE_BOXTYPES_H

#include <FL/Fl.H>

constexpr Fl_Boxtype G_CUSTOM_BORDER_BOX = FL_FREE_BOXTYPE;
constexpr Fl_Boxtype G_CUSTOM_UP_BOX     = static_cast<Fl_Boxtype>(FL_FREE_BOXTYPE + 1);
constexpr Fl_Boxtype G_CUSTOM_DOWN_BOX   = static_cast<Fl_Boxtype>(FL_FREE_BOXTYPE + 3);

void g_customBorderBox(int x, int y, int w, int h, Fl_Color c);
void g_customUpBox(int x, int y, int w, int h, Fl_Color c);
void g_customDownBox(int x, int y, int w, int h, Fl_Color c);

#endif
