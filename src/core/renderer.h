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

#ifndef G_RENDERER_H
#define G_RENDERER_H

namespace giada::m
{
class Sequencer;
class Mixer;
#ifdef WITH_AUDIO_JACK
class JackSynchronizer;
class JackTransport;
#endif
class Renderer
{
public:
#ifdef WITH_AUDIO_JACK
	Renderer(Sequencer&, Mixer&, JackSynchronizer&, JackTransport&);
#else
	Renderer(Sequencer&, Mixer&);
#endif

private:
	Sequencer& m_sequencer;
	Mixer&     m_mixer;
#ifdef WITH_AUDIO_JACK
	JackSynchronizer& m_jackSynchronizer;
	JackTransport&    m_jackTransport;
#endif
};
} // namespace giada::m

#endif
