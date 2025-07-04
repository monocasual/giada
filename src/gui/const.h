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
 * will be useful, but WITHOUT ANY WARRANTY without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------- */

#ifndef G_GUI_CONST_H
#define G_GUI_CONST_H

#include "src/types.h"

namespace giada
{
#define G_COLOR_RED fl_rgb_color(28, 32, 80)
#define G_COLOR_BLUE fl_rgb_color(113, 31, 31)
#define G_COLOR_LIGHT_2 fl_rgb_color(200, 200, 200)
#define G_COLOR_LIGHT_1 fl_rgb_color(170, 170, 170)
#define G_COLOR_GREY_5 fl_rgb_color(110, 110, 110)
#define G_COLOR_GREY_4 fl_rgb_color(78, 78, 78)
#define G_COLOR_GREY_3 fl_rgb_color(54, 54, 54)
#define G_COLOR_GREY_2 fl_rgb_color(37, 37, 37)
#define G_COLOR_GREY_1_5 fl_rgb_color(28, 28, 28)
#define G_COLOR_GREY_1 fl_rgb_color(25, 25, 25)
#define G_COLOR_BLACK fl_rgb_color(0, 0, 0)

constexpr int   G_GUI_FPS            = 30;
constexpr float G_GUI_REFRESH_RATE   = 1 / static_cast<float>(G_GUI_FPS);
constexpr int   G_GUI_FONT_SIZE_BASE = 12;
constexpr int   G_GUI_INNER_MARGIN   = 4;
constexpr int   G_GUI_OUTER_MARGIN   = 8;
constexpr int   G_GUI_UNIT           = 20; // base unit for elements
constexpr int   G_GUI_ZOOM_FACTOR    = 2;

/* -- unique IDs of mainWin's subwindows ------------------------------------ */
constexpr ID WID_BEATS           = 1;
constexpr ID WID_BPM             = 2;
constexpr ID WID_ABOUT           = 3;
constexpr ID WID_FILE_BROWSER    = 4;
constexpr ID WID_CONFIG          = 5;
constexpr ID WID_FX_LIST         = 6;
constexpr ID WID_ACTION_EDITOR   = 7;
constexpr ID WID_SAMPLE_EDITOR   = 8;
constexpr ID WID_KEY_GRABBER     = 10;
constexpr ID WID_SAMPLE_NAME     = 11;
constexpr ID WID_FX_CHOOSER      = 12;
constexpr ID WID_MIDI_INPUT      = 13;
constexpr ID WID_MIDI_OUTPUT     = 14;
constexpr ID WID_MISSING_ASSETS  = 15;
constexpr ID WID_CHANNEL_ROUTING = 16;
constexpr ID WID_FX              = 1000; // Plug-in windows have ID == WID_FX + [pluginId]
} // namespace giada

#endif
