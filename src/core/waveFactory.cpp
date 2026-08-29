/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
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

#include "src/core/waveFactory.h"
#include "audioFileIO.h"
#include "src/core/const.h"
#include "src/core/idManager.h"
#include "src/core/patch.h"
#include "src/core/wave.h"
#include "src/core/waveFx.h"
#include "src/deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include "src/deps/mcl-utils/src/fs.hpp"
#include "src/utils/log.h"
#include <cmath>
#include <fmt/core.h>
#include <memory>
#include <samplerate.h>

namespace utils = mcl::utils;

namespace giada::m::waveFactory
{
namespace
{
IdManager waveId_;

/* -------------------------------------------------------------------------- */

std::string makeWavePath_(const std::string& base, const m::Wave& w, int k)
{
	return utils::fs::join(base, fmt::format("{}-{}{}", w.getBasename(/*ext=*/false), k, w.getExtension()));
}

/* -------------------------------------------------------------------------- */

bool isWavePathUnique_(const m::Wave& skip, const std::string& path,
    const std::vector<std::unique_ptr<Wave>>& waves)
{
	for (const auto& w : waves)
		if (w->id != skip.id && w->getPath() == path)
			return false;
	return true;
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::string makeUniqueWavePath(const std::string& base, const m::Wave& w,
    const std::vector<std::unique_ptr<Wave>>& waves)
{
	std::string path = utils::fs::join(base, w.getBasename(/*ext=*/true));
	if (isWavePathUnique_(w, path, waves))
		return path;

	// TODO - just use a timestamp. e.g. makeWavePath_(..., ..., getTimeStamp())
	int k = 0;
	path  = makeWavePath_(base, w, k);
	while (!isWavePathUnique_(w, path, waves))
		path = makeWavePath_(base, w, k++);

	return path;
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void reset()
{
	waveId_ = IdManager();
}

/* -------------------------------------------------------------------------- */

Result createFromFile(const std::string& path, ID id, int samplerate, Resampler::Quality quality)
{
	auto result = audioFileIO::read(path);
	if (!result)
		return {.status = G_RES_ERR_IO};

	const audioFileIO::Info info   = audioFileIO::getInfo(path);
	mcl::AudioBuffer&       buffer = result.value();

	waveId_.set(id);

	std::unique_ptr<Wave> wave = std::make_unique<Wave>(waveId_.generate(id), std::move(buffer),
	    info.sampleRate, info.format, path);

	if (wave->getBuffer().countChannels() == 1 && !wfx::monoToStereo(*wave))
		return {G_RES_ERR_PROCESSING};

	if (wave->getRate() != samplerate)
	{
		u::log::print("[waveFactory::create] file sample rate ({}) != project sample rate ({}), conversion needed\n",
		    wave->getRate(), samplerate);
		if (resample(*wave.get(), quality, samplerate) != G_RES_OK)
			return {G_RES_ERR_PROCESSING};
	}

	u::log::print("[waveFactory::create] new Wave created, {} frames\n", wave->getBuffer().countFrames());

	return {G_RES_OK, std::move(wave)};
}

/* -------------------------------------------------------------------------- */

std::unique_ptr<Wave> createEmpty(int frames, int channels, int samplerate,
    const std::string& name)
{
	std::unique_ptr<Wave> wave = std::make_unique<Wave>(waveId_.generate());
	wave->alloc(frames, channels, samplerate, G_DEFAULT_BIT_DEPTH, name);
	wave->setLogical(true);

	u::log::print("[waveFactory::createEmpty] new empty Wave created, {} frames\n",
	    wave->getBuffer().countFrames());

	return wave;
}

/* -------------------------------------------------------------------------- */

std::unique_ptr<Wave> createFromWave(const Wave& src, int a, int b)
{
	a = a == -1 ? 0 : a;
	b = b == -1 ? src.getBuffer().countFrames() : b;

	const int channels = src.getBuffer().countChannels();
	const int frames   = b - a;

	std::unique_ptr<Wave> wave = std::make_unique<Wave>(waveId_.generate());
	wave->alloc(frames, channels, src.getRate(), src.getBits(), src.getPath());
	wave->getBuffer().setAll(src.getBuffer(), frames, 0, 0);
	wave->setLogical(true);

	u::log::print("[waveFactory::createFromWave] new Wave created, {} frames\n", frames);

	return wave;
}

/* -------------------------------------------------------------------------- */

std::unique_ptr<Wave> deserializeWave(const Patch::Wave& w, int samplerate, Resampler::Quality quality)
{
	return createFromFile(w.path, w.id, samplerate, quality).wave;
}

const Patch::Wave serializeWave(const Wave& w)
{
	return {w.id, utils::fs::basename(w.getPath())};
}

/* -------------------------------------------------------------------------- */

int resample(Wave& w, Resampler::Quality quality, int samplerate)
{
	float ratio         = samplerate / (float)w.getRate();
	int   newSizeFrames = static_cast<int>(ceil(w.getBuffer().countFrames() * ratio));

	mcl::AudioBuffer newData;
	newData.alloc(newSizeFrames, w.getBuffer().countChannels());

	SRC_DATA src_data;
	src_data.data_in       = w.getBuffer().getDataView().data();
	src_data.input_frames  = w.getBuffer().countFrames();
	src_data.data_out      = newData.getDataView().data();
	src_data.output_frames = newSizeFrames;
	src_data.src_ratio     = ratio;

	u::log::print("[waveFactory::resample] resampling: new size={} frames\n", newSizeFrames);

	int ret = src_simple(&src_data, static_cast<int>(quality), w.getBuffer().countChannels());
	if (ret != 0)
	{
		u::log::print("[waveFactory::resample] resampling error: {}\n", src_strerror(ret));
		return G_RES_ERR_PROCESSING;
	}

	w.replaceData(std::move(newData));
	w.setRate(samplerate);

	return G_RES_OK;
}

/* -------------------------------------------------------------------------- */

int save(const Wave& w, const std::string& path)
{
	return audioFileIO::write(path, w.getBuffer(), w.getRate());
}
} // namespace giada::m::waveFactory
