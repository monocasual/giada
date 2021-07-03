/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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
#include "core/channels/channel.h"
#include "deps/mcl-audio-buffer/src/audioBuffer.hpp"

using namespace mcl;

namespace giada::m::audioReceiver
{
Data::Data(const patch::Channel& p)
: inputMonitor(p.inputMonitor)
, overdubProtection(p.overdubProtection)
{
}

/* -------------------------------------------------------------------------- */

void render(const channel::Data& ch, const AudioBuffer& in)
{
	/* If armed and input monitor is on, copy input buffer to channel buffer: 
	this enables the input monitoring. The channel buffer will be overwritten 
	later on by pluginHost::processStack, so that you would record "clean" audio 
	(i.e. not plugin-processed). */

	if (ch.armed && ch.audioReceiver->inputMonitor)
		ch.buffer->audio.set(in, /*gain=*/1.0f); // add, don't overwrite
}
} // namespace giada::m::audioReceiver