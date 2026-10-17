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

#ifndef G_RENDERING_RENDER_INFO_H
#define G_RENDERING_RENDER_INFO_H

#include "src/types.h"
#include <utility>
#include <vector>

namespace giada::m::rendering
{
/* RenderInfo
    Determines how the render() function should behave.
    Mode::NORMAL - normal rendering, starting at offset 'offset';
    Mode::REWIND - two-step rendering, used when the sample must rewind at some
        point ('offset') in the audio buffer;
    Mode::STOP - abort rendering. The audio buffer is silenced starting at
    'offset'. Also triggers onLastFrame().
    Mode::NONE - no command. */

struct RenderInfo
{
	enum class Mode
	{
		NORMAL,
		REWIND,
		STOP,
		NONE
	};

	Mode  mode   = Mode::NORMAL;
	Frame offset = 0;
};

/* RenderCommands
What each channel has to do in the current audio block (e.g. rewind, stop),
decided by advanceTracks() and read later by renderTracks(). One entry per
channel, identified by its ID. Reserve the capacity at startup: no allocations
allowed on the real-time thread. */

using RenderCommands = std::vector<std::pair<ID, RenderInfo>>;
} // namespace giada::m::rendering

#endif