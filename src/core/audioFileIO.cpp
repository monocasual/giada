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

#include "src/core/audioFileIO.h"
#include "const.h"
#include "src/deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include "src/deps/mcl-utils/src/fs.hpp"
#include "src/utils/log.h"
#include <sndfile.h>

namespace utils = mcl::utils;

namespace giada::m::audioFileIO
{
namespace
{
struct SndFileCloser_
{
	void operator()(SNDFILE* f) const
	{
		if (f)
			sf_close(f);
	}
};

/* SndFileHandle_
RAII-powered SNDFILE handle with a custom deleter (SndFileCloser_). */

using SndFileHandle_ = std::unique_ptr<SNDFILE, SndFileCloser_>;

/* -------------------------------------------------------------------------- */

/* deinterleave_
Scatters an interleaved buffer ([f0c0, f0c1, f1c0, f1c1, ...]) into a planar
mcl::AudioBuffer. */

void deinterleave_(const std::vector<float>& interleaved, mcl::AudioBuffer& out)
{
	const int frames   = out.countFrames();
	const int channels = out.countChannels();

	for (int c = 0; c < channels; ++c)
	{
		mcl::AudioBuffer::ChannelView dst = out.getChannelView(c);
		for (int f = 0; f < frames; ++f)
			dst[f] = interleaved[f * channels + c];
	}
}

/* -------------------------------------------------------------------------- */

/* interleave_
As above, just the opposite. */

std::vector<float> interleave_(const mcl::AudioBuffer& in)
{
	const int frames   = in.countFrames();
	const int channels = in.countChannels();

	std::vector<float> out(static_cast<std::size_t>(frames) * channels);

	for (int c = 0; c < channels; ++c)
	{
		mcl::AudioBuffer::ConstChannelView src = in.getChannelView(c);
		for (int f = 0; f < frames; ++f)
			out[f * channels + c] = src[f];
	}

	return out;
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

std::expected<mcl::AudioBuffer, int> read(const std::string& path)
{
	if (path == "" || utils::fs::isDir(path))
	{
		u::log::print("[audioFileIO::read] Malformed path (was '{}')\n", path);
		return std::unexpected{G_RES_ERR_NO_DATA};
	}

	if (path.size() > FILENAME_MAX)
		return std::unexpected{G_RES_ERR_PATH_TOO_LONG};

	SF_INFO        header{};
	SndFileHandle_ file(sf_open(path.c_str(), SFM_READ, &header));

	if (file == nullptr)
	{
		u::log::print("[audioFileIO::read] Unable to read {}. {}\n", path, sf_strerror(file.get()));
		return std::unexpected{G_RES_ERR_IO};
	}

	if (header.channels > G_MAX_IO_CHANS)
	{
		u::log::print("[audioFileIO::read] Unsupported multi-channel ({}) audio file\n", header.channels);
		return std::unexpected{G_RES_ERR_WRONG_DATA};
	}

	std::vector<float> interleaved(static_cast<std::size_t>(header.frames) * header.channels);
	sf_count_t         framesRead = sf_readf_float(file.get(), interleaved.data(), header.frames);

	if (framesRead != header.frames)
		u::log::print("[audioFileIO::read] Warning: incomplete read!\n");

	mcl::AudioBuffer buffer(static_cast<int>(header.frames), header.channels);
	deinterleave_(interleaved, buffer);

	return buffer;
}

/* -------------------------------------------------------------------------- */

int write(const std::string& path, const mcl::AudioBuffer& buffer, int sampleRate)
{
	SF_INFO info{};
	info.samplerate = sampleRate;
	info.channels   = buffer.countChannels();
	info.format     = SF_FORMAT_WAV | SF_FORMAT_FLOAT;

	SndFileHandle_ file(sf_open(path.c_str(), SFM_WRITE, &info));
	if (!file)
	{
		u::log::print("[audioFileIO::write] Unable to open '{}' for writing\n", path);
		return G_RES_ERR_IO;
	}

	std::vector<float> interleaved   = interleave_(buffer);
	sf_count_t         framesWritten = sf_writef_float(file.get(), interleaved.data(), buffer.countFrames());

	if (framesWritten != buffer.countFrames())
		u::log::print("[audioFileIO::write] Warning: incomplete write!\n");

	return G_RES_OK;
}
} // namespace giada::m::audioFileIO
