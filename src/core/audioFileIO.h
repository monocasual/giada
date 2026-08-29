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

#ifndef G_AUDIO_FILE_IO_H
#define G_AUDIO_FILE_IO_H

#include <expected>
#include <string>

namespace mcl
{
class AudioBuffer;
}

namespace giada::m::audioFileIO
{
struct Info
{
	int sampleRate = 0;
	int format     = 0;
};

Info getInfo(const std::string& path);

std::expected<mcl::AudioBuffer, int> read(const std::string& path);

int write(const std::string& path, const mcl::AudioBuffer&, int sampleRate);

} // namespace giada::m::audioFileIO

#endif
