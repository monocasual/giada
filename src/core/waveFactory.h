/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_WAVE_FACTORY_H
#define G_WAVE_FACTORY_H

#include "core/idManager.h"
#include "core/patch.h"
#include "core/types.h"
#include "core/wave.h"
#include <memory>
#include <string>

namespace giada::m
{
std::string makeUniqueWavePath(const std::string& base, const m::Wave& w,
    const std::vector<std::unique_ptr<Wave>>& waves);

/* -------------------------------------------------------------------------- */

class WaveFactory final
{
public:
	struct Result
	{
		int                   status;
		std::unique_ptr<Wave> wave = nullptr;
	};

	/* reset
    Resets internal ID generator. */

	static void reset();

	/* create
	Creates a new Wave object with data read from file 'path'. Pass id = 0 to 
	auto-generate it. The function converts the Wave sample rate if it doesn't 
	match the desired one as specified in 'samplerate'. */

	static Result createFromFile(const std::string& path, ID id, int samplerate, int quality);

	/* createEmpty
	Creates a new silent Wave object. */

	static std::unique_ptr<Wave> createEmpty(int frames, int channels, int samplerate,
	    const std::string& name);

	/* createFromWave
	Creates a new Wave from an existing one. If specified, copying the data in 
	range a - b. Range is [0, sr.buffer.countFrames()] otherwise. */

	static std::unique_ptr<Wave> createFromWave(const Wave& src, int a = -1, int b = -1);

	/* (de)serializeWave
	Creates a new Wave given the patch raw data and vice versa. */

	static std::unique_ptr<Wave> deserializeWave(const Patch::Wave& w, int samplerate, int quality);
	static const Patch::Wave     serializeWave(const Wave& w);

	/* resample
	Change sample rate of 'w' to the desider value. The 'quality' parameter sets 
	the algorithm to use for the conversion. */

	static int resample(Wave& w, int quality, int samplerate);

	/* save
	Writes Wave data to file 'path'. Only 'wav' format is supported for now. */

	static int save(const Wave& w, const std::string& path);

private:
	static IdManager m_waveId;
};
} // namespace giada::m

#endif
