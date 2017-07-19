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


#include <cmath>
#include <sndfile.h>
#include <samplerate.h>
#include "../utils/log.h"
#include "../utils/fs.h"
#include "const.h"
#include "wave.h"
#include "waveFx.h"
#include "waveManager.h"


namespace giada {
namespace m {
namespace waveManager
{
namespace
{
}; // {anonymous}


/* -------------------------------------------------------------------------- */


int create(const std::string& path, Wave* out)
{
	if (path == "" || gu_isDir(path)) {
		gu_log("[waveManager] file not specified\n");
		return G_RES_ERR_NO_DATA;
	}

	if (path.size() > FILENAME_MAX)
		return G_RES_ERR_PATH_TOO_LONG;

	SF_INFO  header;
	SNDFILE* fileIn = sf_open(path.c_str(), SFM_READ, &header);

	if (fileIn == nullptr) {
		gu_log("[waveManager] unable to read %s. %s\n", path.c_str(), sf_strerror(fileIn));
		return G_RES_ERR_IO;
	}

	if (header.channels > 2) {
		gu_log("[waveManager] unsupported multi-channel sample\n");
		return G_RES_ERR_WRONG_DATA;
	}

	/* Libsndfile's frame structure:

	frame1 = [leftChannel, rightChannel]
	frame2 = [leftChannel, rightChannel]
	... */
	
	int size = header.frames * header.channels;
	float* data = new (std::nothrow) float[size];
	if (data == nullptr) {
		gu_log("[waveManager] unable to allocate memory\n");
		return G_RES_ERR_MEMORY;
	}

	if (sf_read_float(fileIn, data, size) != size)
		gu_log("[waveManager] warning: incomplete read!\n");

	sf_close(fileIn);

	Wave* wave = new Wave(data, size, header.channels, path);	

	if (header.channels == 1 && !wfx_monoToStereo(wave)) {
		delete wave;
		return G_RES_ERR_PROCESSING;
	}

	out = wave;

	return G_RES_OK;
}


/* -------------------------------------------------------------------------- */


int createEmpty(int size, int samplerate, Wave* out)
{
	float* data = new (std::nothrow) float[size];
	if (data == nullptr) {
		gu_log("[waveManager] unable to allocate memory\n");
		return G_RES_ERR_MEMORY;
	}

	out = new Wave(data, size, 2, "");
	out->setLogical(true);	

	return G_RES_OK;
}


/* -------------------------------------------------------------------------- */


int resample(Wave* w, int quality, int samplerate)
{
	float ratio = samplerate / (float) w->getRate();
	int size = ceil(w->getSize() * ratio);
	if (size % 2 != 0)   // libsndfile goes crazy with odd size in case of saving
		size++;

	float* data = new (std::nothrow) float[size];
	if (data == nullptr) {
		gu_log("[waveManager] unable to allocate memory\n");
		return G_RES_ERR_MEMORY;
	}

	SRC_DATA src_data;
	src_data.data_in       = w->getData();
	src_data.input_frames  = w->getSize() / 2;   // in frames, i.e. /2 (stereo)
	src_data.data_out      = data;
	src_data.output_frames = size / 2;           // in frames, i.e. /2 (stereo)
	src_data.src_ratio     = ratio;

	gu_log("[waveManager] resampling: new size=%d (%d frames)\n", size, size / 2);

	int ret = src_simple(&src_data, quality, 2);
	if (ret != 0) {
		gu_log("[waveManager] resampling error: %s\n", src_strerror(ret));
		delete[] data;
		return G_RES_ERR_PROCESSING;
	}

	w->clear();
	w->setData(data);
	w->setSize(size);
	w->setRate(samplerate);

	return G_RES_OK;
}
}}}; // giada::m::waveManager