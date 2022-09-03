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

#ifndef G_CHANNEL_SAMPLE_PLAYER_H
#define G_CHANNEL_SAMPLE_PLAYER_H

#include "core/channels/waveReader.h"
#include "core/const.h"
#include "core/patch.h"
#include "core/sequencer.h"
#include "core/types.h"
#include <functional>

namespace giada::m
{
struct ChannelShared;
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

	SamplePlayer(Resampler* r);
	SamplePlayer(const Patch::Channel& p, float samplerateRatio, Resampler* r, Wave* w);

	bool  hasWave() const;
	bool  hasLogicalWave() const;
	bool  hasEditedWave() const;
	bool  isAnyLoopMode() const;
	ID    getWaveId() const;
	Frame getWaveSize() const;
	Wave* getWave() const;
	void  render(ChannelShared&, Render, bool seqIsRunning) const;

	/* loadWave
	Loads Wave and sets it up (name, markers, ...). Also updates Channel's shared
	state accordingly. Resets begin/end points shift if not specified. */

	void loadWave(ChannelShared&, Wave*, Frame begin = -1, Frame end = -1, Frame shift = -1);

	/* setWave
	Just sets the pointer to a Wave object. Used during de-serialization. The
	ratio is used to adjust begin/end points in case of patch vs. conf sample
	rate mismatch. If nullptr, set the wave to invalid. */

	void setWave(Wave* w, float samplerateRatio);

	/* kickIn
	Starts the player right away at frame 'f'. Used when launching a loop after
	being live recorded. */

	void kickIn(ChannelShared&, Frame f);

	float            pitch;
	SamplePlayerMode mode;
	Frame            shift;
	Frame            begin;
	Frame            end;
	bool             velocityAsVol; // Velocity drives volume
	WaveReader       waveReader;

	/* onLastFrame
	Callback fired when the last frame has been reached. 'natural' == true
	if the rendering has ended because the end of the sample has ben reached. 
	'natural' == false if the rendering has been manually interrupted (by
	a Render::Mode::STOP type). */

	std::function<void(bool natural, bool seqIsRunning)> onLastFrame;

private:
	/* render
	Renders audio into the buffer. Reads audio data from 'tracker' and copies it
	into the audio buffer at position 'offset'. May fire 'onLastFrame' callback
	if the sample end is reached. */

	Frame render(mcl::AudioBuffer&, Frame tracker, Frame offset, ChannelStatus, bool seqIsRunning) const;

	/* stop
	Silences the last part of the audio buffer, starting at 'offset'. Used to
	terminate rendering. It also fire the 'onLastFrame' callback. */

	Frame stop(mcl::AudioBuffer&, Frame offset, bool seqIsRunning) const;

	WaveReader::Result fillBuffer(mcl::AudioBuffer&, Frame start, Frame offset) const;
	bool               shouldLoop(ChannelStatus) const;
};
} // namespace giada::m

#endif
