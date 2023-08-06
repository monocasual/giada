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

#ifndef G_CHANNEL_SAMPLE_PLAYER_H
#define G_CHANNEL_SAMPLE_PLAYER_H

#include "src/core/waveReader.h"
#include "core/types.h"
#include "core/weakAtomic.h"
#include <functional>

namespace giada::m
{
class Channel;
class SamplePlayer final
{
public:
	/* Render
	Determines how the render() function should behave. 
	Mode::NORMAL - normal rendering, starting at offset 'offset';
	Mode::REWIND - two-step rendering, used when the sample must rewind at some
		point ('offset') in the audio buffer;
	Mode::STOP - abort rendering. The audio buffer is silenced starting at
	'offset'. Also triggers onLastFrame(). */

	struct Render
	{
		enum class Mode
		{
			NORMAL,
			REWIND,
			STOP
		};

		Mode  mode   = Mode::NORMAL;
		Frame offset = 0;
	};

	SamplePlayer();

	void render(const Channel&, Render, bool seqIsRunning) const;

	/* onLastFrame
	Callback fired when the last frame has been reached. 'natural' == true
	if the rendering has ended because the end of the sample has ben reached. 
	'natural' == false if the rendering has been manually interrupted (by
	a Render::Mode::STOP type). */

	std::function<void(const Channel&, bool natural, bool seqIsRunning)> onLastFrame;

private:
	/* render
	Renders audio into the buffer. Reads audio data from 'tracker' and copies it
	into the audio buffer at position 'offset'. May fire 'onLastFrame' callback
	if the sample end is reached. */

	Frame render(const Channel&, mcl::AudioBuffer&, Frame tracker, Frame offset, bool seqIsRunning) const;

	/* stop
	Silences the last part of the audio buffer, starting at 'offset'. Used to
	terminate rendering. It also fire the 'onLastFrame' callback. */

	void stop(const Channel&, mcl::AudioBuffer&, Frame offset, bool seqIsRunning) const;

	bool shouldLoop(SamplePlayerMode, ChannelStatus) const;

	WaveReader m_waveReader;
};
} // namespace giada::m

#endif
