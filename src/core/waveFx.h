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

#ifndef G_WAVE_FX_H
#define G_WAVE_FX_H

#include "src/core/types.h"

namespace giada::m
{
class Wave;
}

namespace giada::m::wfx
{
/* Windows fix */
#ifdef _WIN32
#undef IN
#undef OUT
#endif
enum class Fade
{
	IN,
	OUT
};

/* monoToStereo
Converts a 1-channel Wave to a 2-channels wave. */

int monoToStereo(Wave& w);

/* normalize
Normalizes the wave in range a-b by altering values in memory. */

void normalize(Wave& w, int a, int b);

void silence(Wave& w, int a, int b);
void cut(Wave& w, int a, int b);
void trim(Wave& w, int a, int b);

/* paste
Pastes Wave 'src' into Wave 'dest', starting from frame 'a'. */

void paste(const Wave& src, Wave& dest, Frame a);

/* fade
Fades in or fades out selection. Can be Fade::IN or Fade::OUT. */

void fade(Wave& w, int a, int b, Fade type);

/* smooth
Smooth edges of selection. */

void smooth(Wave& w, int a, int b);

/* reverse
Flips Wave's data. */

void reverse(Wave& v, Frame a, Frame b);

void shift(Wave& w, Frame offset);
} // namespace giada::m::wfx

#endif
