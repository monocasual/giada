/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_CHANNEL_AUDIO_RECEIVER_H
#define G_CHANNEL_AUDIO_RECEIVER_H

#include "core/patch.h"

namespace mcl
{
class AudioBuffer;
}

namespace giada::m
{
class AudioReceiver final
{
public:
	AudioReceiver() = default;
	AudioReceiver(const Patch::Channel& p);
	AudioReceiver(const AudioReceiver& o) = default;

	void render(const mcl::AudioBuffer& in, mcl::AudioBuffer& out, bool armed) const;

	bool inputMonitor;
	bool overdubProtection;
};
} // namespace giada::m

#endif
