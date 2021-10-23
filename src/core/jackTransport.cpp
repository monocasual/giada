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

#include "jackTransport.h"
#ifdef WITH_AUDIO_JACK
#include <jack/intclient.h>
#include <jack/transport.h>
#endif

namespace giada::m
{
bool JackTransport::State::operator!=(const State& o) const
{
	return !(running == o.running && bpm == o.bpm && frame == o.frame);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

JackTransport::JackTransport()
#ifdef WITH_AUDIO_JACK
: m_jackHandle(nullptr)
#endif
{
}

/* -------------------------------------------------------------------------- */

bool JackTransport::start() const
{
#ifdef WITH_AUDIO_JACK
	if (m_jackHandle == nullptr)
		return false;
	jack_transport_start(m_jackHandle);
	return true;
#else
	return false;
#endif
}

/* -------------------------------------------------------------------------- */

bool JackTransport::stop() const
{
#ifdef WITH_AUDIO_JACK
	if (m_jackHandle == nullptr)
		return false;
	jack_transport_stop(m_jackHandle);
	return true;
#else
	return false;
#endif
}

/* -------------------------------------------------------------------------- */

bool JackTransport::setPosition(uint32_t frame) const
{
#ifdef WITH_AUDIO_JACK
	if (m_jackHandle == nullptr)
		return false;
	jack_position_t position;
	jack_transport_query(m_jackHandle, &position);
	position.frame = frame;
	jack_transport_reposition(m_jackHandle, &position);
	return true;
#else
	return false;
#endif
}

/* -------------------------------------------------------------------------- */

bool JackTransport::setBpm(double bpm) const
{
#ifdef WITH_AUDIO_JACK
	if (m_jackHandle == nullptr)
		return false;
	jack_position_t position;
	jack_transport_query(m_jackHandle, &position);
	position.valid            = jack_position_bits_t::JackPositionBBT;
	position.bar              = 0; // no such info from Giada
	position.beat             = 0; // no such info from Giada
	position.tick             = 0; // no such info from Giada
	position.beats_per_minute = bpm;
	jack_transport_reposition(m_jackHandle, &position);
	return true;
#else
	return false;
#endif
}

/* -------------------------------------------------------------------------- */

JackTransport::State JackTransport::getState() const
{
#ifdef WITH_AUDIO_JACK
	if (m_jackHandle == nullptr)
		return {};

	jack_position_t        position;
	jack_transport_state_t ts = jack_transport_query(m_jackHandle, &position);

	return {
	    ts != JackTransportStopped,
	    position.beats_per_minute,
	    position.frame};
#else
	return {};
#endif
}

/* -------------------------------------------------------------------------- */

#ifdef WITH_AUDIO_JACK
void JackTransport::setHandle(jack_client_t* h)
{
	m_jackHandle = h;
}
#endif
} // namespace giada::m
