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

#include "src/utils/string.h"
#include <climits>
#include <cstdarg>
#include <cstddef>
#include <iomanip>
#include <memory>

namespace giada::u::string
{
std::string toString(Thread t)
{
	switch (t)
	{
	case Thread::MAIN:
		return "MAIN";
	case Thread::MIDI:
		return "MIDI";
	case Thread::AUDIO:
		return "AUDIO (rt)";
	case Thread::EVENTS:
		return "EVENTS";
	default:
		return "(unknown)";
	}
}

/* -------------------------------------------------------------------------- */

std::string toString(RtAudio::Api api)
{
	switch (api)
	{
	case RtAudio::Api::LINUX_ALSA:
		return "ALSA";
	case RtAudio::Api::LINUX_PULSE:
		return "PulseAudio";
	case RtAudio::Api::UNIX_JACK:
		return "JACK";
	case RtAudio::Api::MACOSX_CORE:
		return "CoreAudio";
	case RtAudio::Api::WINDOWS_WASAPI:
		return "WASAPI";
	case RtAudio::Api::WINDOWS_ASIO:
		return "ASIO";
	case RtAudio::Api::WINDOWS_DS:
		return "DirectSound";
	case RtAudio::Api::RTAUDIO_DUMMY:
		return "Dummy";
	default:
		return "(unknown)";
	}
}

/* -------------------------------------------------------------------------- */

std::string toString(SamplePlayerMode mode)
{
	switch (mode)
	{
	case SamplePlayerMode::LOOP_BASIC:
		return "LOOP_BASIC";
	case SamplePlayerMode::LOOP_ONCE:
		return "LOOP_ONCE";
	case SamplePlayerMode::LOOP_REPEAT:
		return "LOOP_REPEAT";
	case SamplePlayerMode::LOOP_ONCE_BAR:
		return "LOOP_ONCE_BAR";
	case SamplePlayerMode::SINGLE_BASIC:
		return "SINGLE_BASIC";
	case SamplePlayerMode::SINGLE_PRESS:
		return "SINGLE_PRESS";
	case SamplePlayerMode::SINGLE_RETRIG:
		return "SINGLE_RETRIG";
	case SamplePlayerMode::SINGLE_ENDLESS:
		return "SINGLE_ENDLESS";
	case SamplePlayerMode::SINGLE_BASIC_PAUSE:
		return "SINGLE_BASIC_PAUSE";
	default:
		return "(unknown)";
	}
}

/* -------------------------------------------------------------------------- */

std::string toString(ChannelType type)
{
	switch (type)
	{
	case ChannelType::SAMPLE:
		return "SAMPLE";
	case ChannelType::MIDI:
		return "MIDI";
	case ChannelType::MASTER:
		return "MASTER";
	case ChannelType::PREVIEW:
		return "PREVIEW";
	case ChannelType::GROUP:
		return "GROUP";
	default:
		return "(unknown)";
	}
}
} // namespace giada::u::string
