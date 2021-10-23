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

#ifndef G_JACK_TRANSPORT_H
#define G_JACK_TRANSPORT_H

#ifdef WITH_AUDIO_JACK
#include <jack/jack.h>
#endif
#include <cstdint>

namespace giada::m
{
class JackTransport final
{
public:
	struct State
	{
		bool     running;
		double   bpm;
		uint32_t frame;

		bool operator!=(const State& o) const;
	};

	JackTransport();

	bool  start() const;
	bool  stop() const;
	bool  setPosition(uint32_t frame) const;
	bool  setBpm(double bpm) const;
	State getState() const;

#ifdef WITH_AUDIO_JACK
	void setHandle(jack_client_t*);
#endif

private:
	/* m_jackHandle
	Optional handle to JACK. If nullptr the JackTransport class is not
	initialized and all public transport methods above will return false. This
	is useful when you are on a platform that supports JACK (e.g. Linux) but
	the JACK API is currently not selected. */

#ifdef WITH_AUDIO_JACK
	jack_client_t* m_jackHandle;
#endif
};
} // namespace giada::m

#endif
