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


#ifndef G_WAVE_MANAGER_H
#define G_WAVE_MANAGER_H


#include <string>


class Wave;


namespace giada {
namespace m {
namespace waveManager
{
/* create
Creates a new Wave object with data read from file 'path'. */

int create(const std::string& path, Wave* out);

/* createEmpty
Creates a new silent Wave object. Note: 'size' must take 2 channels into account
(stereo). */

int createEmpty(int size, int samplerate, Wave* out);
int resample(Wave* w, int quality, int samplerate); 

}}}; // giada::m::waveManager

#endif