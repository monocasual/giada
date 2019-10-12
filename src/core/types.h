/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#ifndef G_TYPES_H
#define G_TYPES_H


namespace giada
{
using ID    = int;
using Pixel = int;
using Frame = int;

enum class ClockStatus { STOPPED, WAITING, RUNNING };

enum class ChannelType : int { SAMPLE = 1, MIDI, MASTER, PREVIEW };

enum class ChannelStatus : int
{
	ENDING = 1, WAIT, PLAY, OFF, EMPTY, MISSING, WRONG  
};

enum class ChannelMode : int
{
	LOOP_BASIC = 1, LOOP_ONCE, LOOP_REPEAT, LOOP_ONCE_BAR,
	SINGLE_BASIC, SINGLE_PRESS, SINGLE_RETRIG, SINGLE_ENDLESS
};

enum class RecTriggerMode : int { NORMAL = 0, SIGNAL };

enum class PreviewMode : int { NONE = 0, NORMAL, LOOP };
enum class EventType : int { AUTO = 0, MANUAL };
};


#endif
