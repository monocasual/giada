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
 * will be useful, but WITHOUT ANY WARRANTY; without even the implied
 * warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Giada - Your Hardcore Loopmachine. If not, see
 * <http://www.gnu.org/licenses/>.
 *
 * -------------------------------------------------------------------------- */

#ifdef WITH_AUDIO_JACK

#ifndef G_JACK_SYNCHRONIZER_H
#define G_JACK_SYNCHRONIZER_H

#include "core/jackTransport.h"
#include <functional>

namespace giada::m::kernelAudio
{
struct JackState;
}

namespace giada::m
{
class KernelMidi;
class JackSynchronizer final
{
public:
	JackSynchronizer();

	/* recvJackSync
	Receives a new JACK state. Called by Kernel Audio on each audio block. */

	void recvJackSync(const JackTransport::State&) const;

	/* onJack[...]
	Callbacks called when something happens in the JACK state. */

	std::function<void()>      onJackRewind;
	std::function<void(float)> onJackChangeBpm;
	std::function<void()>      onJackStart;
	std::function<void()>      onJackStop;

private:
	mutable JackTransport::State m_jackStatePrev;
};
} // namespace giada::m

#endif

#endif