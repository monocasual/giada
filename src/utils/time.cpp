/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * utils
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/utils/time.h"
#include <chrono>
#include <thread>

namespace giada::u::time
{
void sleep(int millisecs)
{
	std::this_thread::sleep_for(std::chrono::milliseconds(millisecs));
}

/* -------------------------------------------------------------------------- */

Frame beatToFrame(int beat, int sampleRate, float bpm)
{
	return static_cast<Frame>((sampleRate * (60.0f / bpm)) * beat);
}

/* -------------------------------------------------------------------------- */

int frameToBeat(Frame frame, int sampleRate, float bpm)
{
	return static_cast<int>(frame / (sampleRate * (60.0f / bpm)));
}
} // namespace giada::u::time
