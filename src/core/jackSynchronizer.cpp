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

#ifdef WITH_AUDIO_JACK

#include "core/jackSynchronizer.h"
#include "core/conf.h"
#include "core/kernelAudio.h"
#include "core/kernelMidi.h"
#include "core/model/model.h"
#include "utils/log.h"

namespace giada::m
{
JackSynchronizer::JackSynchronizer()
: onJackRewind(nullptr)
, onJackChangeBpm(nullptr)
, onJackStart(nullptr)
, onJackStop(nullptr)
{
}

/* -------------------------------------------------------------------------- */

void JackSynchronizer::recvJackSync(const JackTransport::State& state)
{
	assert(onJackRewind != nullptr);
	assert(onJackChangeBpm != nullptr);
	assert(onJackStart != nullptr);
	assert(onJackStop != nullptr);

	JackTransport::State jackStateCurr = state;

	if (jackStateCurr != m_jackStatePrev)
	{
		if (jackStateCurr.frame != m_jackStatePrev.frame && jackStateCurr.frame == 0)
		{
			G_DEBUG("JackState received - rewind to frame 0", );
			onJackRewind();
		}

		// jackStateCurr.bpm == 0 if JACK doesn't send that info
		if (jackStateCurr.bpm != m_jackStatePrev.bpm && jackStateCurr.bpm > 1.0f)
		{
			G_DEBUG("JackState received - bpm={}", jackStateCurr.bpm);
			onJackChangeBpm(jackStateCurr.bpm);
		}

		if (jackStateCurr.running != m_jackStatePrev.running)
		{
			G_DEBUG("JackState received - running={}", jackStateCurr.running);
			jackStateCurr.running ? onJackStart() : onJackStop();
		}
	}

	m_jackStatePrev = jackStateCurr;
}
} // namespace giada::m

#endif