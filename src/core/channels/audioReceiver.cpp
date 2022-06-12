/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "audioReceiver.h"
#include "deps/mcl-audio-buffer/src/audioBuffer.hpp"

namespace giada::m
{
AudioReceiver::AudioReceiver(const Patch::Channel& p)
: inputMonitor(p.inputMonitor)
, overdubProtection(p.overdubProtection)
{
}

/* -------------------------------------------------------------------------- */

void AudioReceiver::render(const mcl::AudioBuffer& in, mcl::AudioBuffer& out, bool armed) const
{
	/* If armed and input monitor is on, copy input buffer to channel buffer: 
	this enables the input monitoring. The channel buffer will be overwritten 
	later on by pluginHost::processStack, so that you would record "clean" audio 
	(i.e. not plugin-processed). */

	if (armed && inputMonitor)
		out.set(in, /*gain=*/1.0f); // add, don't overwrite
}
} // namespace giada::m
