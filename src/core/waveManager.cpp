/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
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


#include <cmath>
#include <sndfile.h>
#include <samplerate.h>
#include "../utils/log.h"
#include "../utils/fs.h"
#include "const.h"
#include "wave.h"
#include "waveFx.h"
#include "waveManager.h"


using std::string;


namespace giada {
namespace m {
namespace waveManager
{
namespace
{
int getBits(SF_INFO& header)
{
	if      (header.format & SF_FORMAT_PCM_S8)
		return 8;
	else if (header.format & SF_FORMAT_PCM_16)
		return 16;
	else if (header.format & SF_FORMAT_PCM_24)
		return 24;
	else if (header.format & SF_FORMAT_PCM_32)
		return 32;
	else if (header.format & SF_FORMAT_PCM_U8)
		return 8;
	else if (header.format & SF_FORMAT_FLOAT)
		return 32;
	else if (header.format & SF_FORMAT_DOUBLE)
		return 64;
	return 0;
}
}; // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


int create(const string& path, Wave** out)
{
	if (path == "" || gu_isDir(path)) {
		gu_log("[waveManager::create] malformed path (was '%s')\n", path.c_str());
		return G_RES_ERR_NO_DATA;
	}

	if (path.size() > FILENAME_MAX)
		return G_RES_ERR_PATH_TOO_LONG;

	SF_INFO header;
	SNDFILE* fileIn = sf_open(path.c_str(), SFM_READ, &header);

	if (fileIn == nullptr) {
		gu_log("[waveManager::create] unable to read %s. %s\n", path.c_str(), sf_strerror(fileIn));
		return G_RES_ERR_IO;
	}

	if (header.channels > 2) {
		gu_log("[waveManager::create] unsupported multi-channel sample\n");
		return G_RES_ERR_WRONG_DATA;
	}

	/* Libsndfile's frame structure:

	frame1 = [leftChannel, rightChannel]
	frame2 = [leftChannel, rightChannel]
	... */

	int size = header.frames * header.channels;
	float* data = new (std::nothrow) float[size];
	if (data == nullptr) {
		gu_log("[waveManager::create] unable to allocate memory\n");
		return G_RES_ERR_MEMORY;
	}

	if (sf_read_float(fileIn, data, size) != size)
		gu_log("[waveManager::create] warning: incomplete read!\n");

	sf_close(fileIn);

	Wave* wave = new Wave(data, size, header.channels, header.samplerate,
		getBits(header), path);

	if (header.channels == 1 && !wfx::monoToStereo(wave)) {
		delete wave;
		return G_RES_ERR_PROCESSING;
	}

	*out = wave;

	gu_log("[waveManager::create] new Wave created, %d frames\n", wave->getSize());

	return G_RES_OK;
}


/* -------------------------------------------------------------------------- */


int createEmpty(int size, int samplerate, const string& name, Wave** out)
{
	float* data = new (std::nothrow) float[size];
	if (data == nullptr) {
		gu_log("[waveManager::createEmpty] unable to allocate memory\n");
		return G_RES_ERR_MEMORY;
	}

	Wave* wave = new Wave(data, size, G_DEFAULT_AUDIO_CHANS, samplerate, 
		G_DEFAULT_BIT_DEPTH, "");
	wave->setLogical(true);

	*out = wave;

	gu_log("[waveManager::createEmpty] new empty Wave created, %d frames\n", size);

	return G_RES_OK;
}


/* -------------------------------------------------------------------------- */


int createFromWave(const Wave* src, int a, int b, Wave** out)
{
	int numChans = src->getChannels();
	int size = (b - a) * numChans;
	float* data = new (std::nothrow) float[size];
	if (data == nullptr) {
		gu_log("[waveManager::createFromWave] unable to allocate memory\n");
		return G_RES_ERR_MEMORY;
	}

	std::copy(src->getData() + (a*numChans), src->getData() + (b*numChans), data);

	Wave* wave = new Wave(data, size, numChans, src->getRate(),
		src->getBits(), src->getPath());
	wave->setLogical(true);

	*out = wave;

	gu_log("[waveManager::createFromWave] new Wave created, %d frames\n", size);

	return G_RES_OK;
}


/* -------------------------------------------------------------------------- */


int resample(Wave* w, int quality, int samplerate)
{
	float ratio = samplerate / (float) w->getRate();
	int newSizeFrames  = ceil(w->getSize() * ratio);
	int newSizeSamples = newSizeFrames * w->getChannels();

	float* newData = new (std::nothrow) float[newSizeSamples];
	if (newData == nullptr) {
		gu_log("[waveManager::resample] unable to allocate memory\n");
		return G_RES_ERR_MEMORY;
	}

	SRC_DATA src_data;
	src_data.data_in       = w->getData();
	src_data.input_frames  = w->getSize();
	src_data.data_out      = newData;
	src_data.output_frames = newSizeFrames;
	src_data.src_ratio     = ratio;

	gu_log("[waveManager::resample] resampling: new size=%d (%d frames)\n",
		newSizeSamples, newSizeFrames);

	int ret = src_simple(&src_data, quality, w->getChannels());
	if (ret != 0) {
		gu_log("[waveManager::resample] resampling error: %s\n", src_strerror(ret));
		delete[] newData;
		return G_RES_ERR_PROCESSING;
	}

	w->free();
	w->setData(newData, newSizeSamples);
	w->setRate(samplerate);

	return G_RES_OK;
}


/* -------------------------------------------------------------------------- */


int save(Wave* w, const string& path)
{
	SF_INFO header;
	header.samplerate = w->getRate();
	header.channels   = w->getChannels();
	header.format     = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

	SNDFILE* file = sf_open(path.c_str(), SFM_WRITE, &header);
	if (file == nullptr) {
		gu_log("[waveManager::save] unable to open %s for exporting: %s\n",
			path.c_str(), sf_strerror(file));
		return G_RES_ERR_IO;
	}

	if (sf_writef_float(file, w->getData(), w->getSize()) != w->getSize())
		gu_log("[waveManager::save] warning: incomplete write!\n");

	sf_close(file);

	w->setLogical(false);
	w->setEdited(false);

	return G_RES_OK;
}
}}}; // giada::m::waveManager
