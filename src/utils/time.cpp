/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * utils
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

#include "src/utils/time.h"
#include <chrono>
#include <cmath>
#include <thread>

namespace giada::u::time
{
namespace
{
double frameToTickAsDouble_(Frame frame, int sampleRate, float bpm)
{
	// ticks = frame * (ppq * bpm) / (sampleRate * 60.0);
	return static_cast<double>(frame) *
	       (static_cast<double>(G_PPQ) * bpm) /
	       (sampleRate * 60.0);
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
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

/* -------------------------------------------------------------------------- */

Frame tickToFrame(Tick tick, int sampleRate, float bpm)
{
	// frames = ticks * sampleRate * 60.0 / (ppq * bpm);
	const double frameDouble = static_cast<double>(tick.value()) *
	                           static_cast<double>(sampleRate) * 60.0 /
	                           (static_cast<double>(G_PPQ) * bpm);
	return static_cast<Frame>(std::llround(frameDouble));
}

/* -------------------------------------------------------------------------- */

Tick frameToTickFloor(Frame frame, int sampleRate, float bpm)
{
	const double tickDouble = frameToTickAsDouble_(frame, sampleRate, bpm);
	return Tick{static_cast<Tick::Value>(std::floor(tickDouble))};
}

Tick frameToTickCeil(Frame frame, int sampleRate, float bpm)
{
	const double tickDouble = frameToTickAsDouble_(frame, sampleRate, bpm);
	return Tick{static_cast<Tick::Value>(std::ceil(tickDouble))};
}
} // namespace giada::u::time
