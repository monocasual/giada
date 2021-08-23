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

#ifdef WITH_AUDIO_JACK

#ifndef G_JACK_TRANSPORT_H
#define G_JACK_TRANSPORT_H

#include <jack/jack.h>

namespace giada
{
class JackTransport
{
public:
	struct State
	{
		bool     running;
		double   bpm;
		uint32_t frame;

		bool operator!=(const State& o) const;
	};

	JackTransport(jack_client_t&);

	void  start();
	void  stop();
	void  setPosition(uint32_t frame);
	void  setBpm(double bpm);
	State getState();

private:
	jack_client_t& m_jackHandle;
};
} // namespace giada

#endif

#endif
