/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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
#include <memory>
#include "core/types.h"


namespace giada {
namespace m 
{
class Wave;
namespace patch
{
struct Wave;
}
namespace waveManager
{
struct Result
{
    int status;
    std::unique_ptr<Wave> wave = nullptr;
};
/* init
Initializes internal data. */
	
void init();

/* create
Creates a new Wave object with data read from file 'path'. Pass id = 0 to 
auto-generate it. The function converts the Wave sample rate if it doesn't match
the desired one as specified in 'samplerate'. */

Result createFromFile(const std::string& path, ID id, int samplerate, int quality);

/* createEmpty
Creates a new silent Wave object. */

std::unique_ptr<Wave> createEmpty(int frames, int channels, int samplerate, 
    const std::string& name);

/* createFromWave
Creates a new Wave from an existing one, copying the data in range a - b. */

std::unique_ptr<Wave> createFromWave(const Wave& src, int a, int b);

/* (de)serializeWave
Creates a new Wave given the patch raw data and vice versa. */

std::unique_ptr<Wave> deserializeWave(const patch::Wave& w, int samplerate, int quality);
const patch::Wave     serializeWave(const Wave& w);

/* resample
Change sample rate of 'w' to the desider value. The 'quality' parameter sets the
algorithm to use for the conversion. */

int resample(Wave& w, int quality, int samplerate); 

/* save
Writes Wave data to file 'path'. Only 'wav' format is supported for now. */

int save(const Wave& w, const std::string& path);

}}} // giada::m::waveManager


#endif
