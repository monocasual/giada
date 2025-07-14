/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
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
 * will be useful, but WITHOUT ANY WARRANTY without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------- */

#ifndef G_CORE_TYPES_H
#define G_CORE_TYPES_H

namespace giada
{
using Pixel = int;

enum class Thread
{
	MAIN,
	MIDI,
	AUDIO,
	EVENTS
};

/* Windows fix */
#ifdef _WIN32
#undef VOID
#endif
enum class SeqStatus
{
	STOPPED,
	WAITING,
	RUNNING,
	ON_BEAT,
	ON_BAR,
	ON_FIRST_BEAT,
	VOID
};

enum class ChannelType : int
{
	SAMPLE = 1,
	MIDI,
	MASTER,
	PREVIEW,
	GROUP
};

enum class ChannelStatus : int
{
	ENDING = 1,
	WAIT,
	PLAY,
	OFF,
	EMPTY,
	MISSING,
	WRONG
};

enum class SamplePlayerMode : int
{
	LOOP_BASIC = 1,
	LOOP_ONCE,
	LOOP_REPEAT,
	LOOP_ONCE_BAR,
	SINGLE_BASIC,
	SINGLE_PRESS,
	SINGLE_RETRIG,
	SINGLE_ENDLESS,
	SINGLE_BASIC_PAUSE
};

enum class RecTriggerMode : int
{
	NORMAL = 0,
	SIGNAL
};

enum class InputRecMode : int
{
	RIGID = 0,
	FREE
};

/* Peak
Audio peak information for two In/Out channels. */

struct Peak
{
	float left;
	float right;
};
} // namespace giada

#endif
