/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * waveFx
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013 Giovanni A. Zuliani | Monocasual
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
 * ------------------------------------------------------------------ */


#ifndef WAVEFX_H
#define WAVEFX_H

#include <math.h>
#include "wave.h"
#include "mixer.h"

/* normalizeSoft
 * normalize the wave by returning the dB value for the boost volume. It
 * doesn't touch the memory. */

float wfx_normalizeSoft(class Wave *w);

bool wfx_monoToStereo(class Wave *w);

void wfx_silence(class Wave *w, unsigned a, unsigned b);

int wfx_cut(int ch, unsigned a, unsigned b);

int wfx_trim(int ch, unsigned a, unsigned b);

#endif
