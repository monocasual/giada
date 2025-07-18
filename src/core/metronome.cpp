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

#include "src/core/metronome.h"
#include "src/deps/mcl-audio-buffer/src/audioBuffer.hpp"

namespace giada::m
{
void Metronome::trigger(Click c, Frame o) const
{
	m_rendering = true;
	m_click     = c;
	m_offset    = o;
}

/* -------------------------------------------------------------------------- */

void Metronome::render(mcl::AudioBuffer& outBuf) const
{
	const float* data = m_click == Click::BEAT ? beat : bar;
	for (Frame f = m_offset; f < outBuf.countFrames() && m_rendering; f++)
	{
		for (int c = 0; c < outBuf.countChannels(); c++)
			outBuf[f][c] += data[m_tracker];
		m_tracker = (m_tracker + 1) % CLICK_SIZE;
		if (m_tracker == 0)
			m_rendering = false;
	}
	m_offset = 0;
}
} // namespace giada::m