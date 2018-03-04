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

	if (header.channels > G_MAX_IO_CHANS) {
		gu_log("[waveManager::create] unsupported multi-channel sample\n");
		return G_RES_ERR_WRONG_DATA;
	}

	Wave* wave = new Wave();
	if (!wave->alloc(header.frames, header.channels, header.samplerate, getBits(header), path)) {
		gu_log("[waveManager::create] unable to allocate memory\n");
		delete wave;
		return G_RES_ERR_MEMORY;
	}

	if (sf_readf_float(fileIn, wave->getFrame(0), header.frames) != header.frames)
		gu_log("[waveManager::create] warning: incomplete read!\n");

	sf_close(fileIn);

	if (header.channels == 1 && !wfx::monoToStereo(*wave)) {
		delete wave;
		return G_RES_ERR_PROCESSING;
	}

	*out = wave;

	gu_log("[waveManager::create] new Wave created, %d frames\n", wave->getSize());

	return G_RES_OK;
}


/* -------------------------------------------------------------------------- */


int createEmpty(int frames, int channels, int samplerate, const string& name, 
	Wave** out)
{
	Wave* wave = new Wave();
	if (!wave->alloc(frames, channels, samplerate, G_DEFAULT_BIT_DEPTH, "")) {
		gu_log("[waveManager::createEmpty] unable to allocate memory\n");
		delete wave;
		return G_RES_ERR_MEMORY;
	}

	wave->setLogical(true);

	*out = wave;

	gu_log("[waveManager::createEmpty] new empty Wave created, %d frames\n", 
		wave->getSize());

	return G_RES_OK;
}


/* -------------------------------------------------------------------------- */


int createFromWave(const Wave* src, int a, int b, Wave** out)
{
	int channels = src->getChannels();
	int frames   = b - a;

	Wave* wave = new Wave();
	if (!wave->alloc(frames, channels, src->getRate(), src->getBits(), src->getPath())) {
		gu_log("[waveManager::createFromWave] unable to allocate memory\n");
		delete wave;
		return G_RES_ERR_MEMORY;
	}

	wave->copyData(src->getFrame(a), frames);
	wave->setLogical(true);

	*out = wave;

	gu_log("[waveManager::createFromWave] new Wave created, %d frames\n", frames);

	return G_RES_OK;
}


/* -------------------------------------------------------------------------- */


int resample(Wave* w, int quality, int samplerate)
{
	float ratio = samplerate / (float) w->getRate();
	int newSizeFrames = ceil(w->getSize() * ratio);

	AudioBuffer newData;
	if (!newData.alloc(newSizeFrames, w->getChannels())) {
		gu_log("[waveManager::resample] unable to allocate memory\n");
		return G_RES_ERR_MEMORY;
	}

	SRC_DATA src_data;
	src_data.data_in       = w->getFrame(0);
	src_data.input_frames  = w->getSize();
	src_data.data_out      = newData[0];
	src_data.output_frames = newSizeFrames;
	src_data.src_ratio     = ratio;

	gu_log("[waveManager::resample] resampling: new size=%d frames\n", newSizeFrames);

	int ret = src_simple(&src_data, quality, w->getChannels());
	if (ret != 0) {
		gu_log("[waveManager::resample] resampling error: %s\n", src_strerror(ret));
		return G_RES_ERR_PROCESSING;
	}

	w->moveData(newData);
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

	if (sf_writef_float(file, w->getFrame(0), w->getSize()) != w->getSize())
		gu_log("[waveManager::save] warning: incomplete write!\n");

	sf_close(file);

	w->setLogical(false);
	w->setEdited(false);

	return G_RES_OK;
}
}}}; // giada::m::waveManager
