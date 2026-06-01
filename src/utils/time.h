/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * utils
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_UTILS_TIME_H
#define G_UTILS_TIME_H

#include "src/tick.h"
#include "src/types.h"

namespace giada::u::time
{
/* beatToFrame
Returns the frame a beat corresponds to. */

Frame beatToFrame(int beat, int sampleRate, float bpm);

/* frameToBeat
Returns the beat a frame corresponds to. */

int frameToBeat(Frame frame, int sampleRate, float bpm);

/* tickToFrame
Converts Tick -> Frame. */
Frame tickToFrame(Tick, int sampleRate, float bpm);

/* frameToTick[...]
Converts Frame -> Tick. Provides floor and ceil rounding modes since the frame -> tick
conversion is not always exact: the result may lie between two integer tick
values. These variants expose the two possible integer bounds, floor and ceil,
used when converting ranges to avoid skipping information at boundaries. Use the
round one instead when converting a single frame to tick. */

Tick frameToTickFloor(Frame, int sampleRate, float bpm);
Tick frameToTickCeil(Frame, int sampleRate, float bpm);
Tick frameToTickRound(Frame, int sampleRate, float bpm);

/* frameRangeToTickRange
Same as frameToTick[...], but on ranges. Uses floor for 'a' point and ceil for
'b' point to maka sure the tick interval fully covers everything touched by the
frame range. */

TickRange frameRangeToTickRange(FrameRange, int sampleRate, float bpm);

/* tickToPixel, pixelToTick
Converts Tick <-> pixel. */

int  tickToPixel(Tick, double ratio);
Tick pixelToTick(int pixel, double ratio);
} // namespace giada::u::time

#endif