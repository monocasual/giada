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

#ifndef G_MODEL_SEQUENCER_H
#define G_MODEL_SEQUENCER_H

#include "src/core/const.h"
#include "src/core/types.h"
#include "src/core/weakAtomic.h"
#include "src/deps/mcl-audio-buffer/src/audioBuffer.hpp"

namespace giada::m::model
{
class Sequencer
{
	friend class Model;
	friend class Shared;

public:
	/* isRunning
	When sequencer is actually moving forward, i.e. SeqStatus == RUNNING. */

	bool isRunning() const;

	/* isActive
	Sequencer is enabled, but might be in wait mode, i.e. SeqStatus == RUNNING or
	SeqStatus == WAITING. */

	bool isActive() const;

	/* canQuantize
	Sequencer can quantize only if it's running and quantizer is enabled. */

	bool canQuantize() const;

	bool a_isOnBar() const;
	bool a_isOnBeat() const;
	bool a_isOnFirstBeat() const;

	Frame a_getCurrentFrame() const;
	Frame a_getCurrentBeat() const;
	float a_getCurrentSecond(int sampleRate) const;

	/* getMaxFramesInLoop
	Returns how many frames the current loop length might contain at the slowest
	speed possible (G_MIN_BPM). */

	int getMaxFramesInLoop(int sampleRate) const;

	void a_setCurrentFrame(Frame f, int sampleRate) const;
	void a_setCurrentBeat(int b, int sampleRate) const;

	SeqStatus   status       = SeqStatus::STOPPED;
	int         framesInLoop = 0;
	int         framesInBar  = 0;
	int         framesInBeat = 0;
	int         framesInSeq  = 0;
	int         bars         = G_DEFAULT_BARS;
	int         beats        = G_DEFAULT_BEATS;
	float       bpm          = G_DEFAULT_BPM;
	int         quantize     = G_DEFAULT_QUANTIZE;
	bool        metronome    = false;
	std::size_t currentScene = 0;

private:
	struct Shared
	{
		WeakAtomic<Frame> currentFrame = 0;
		WeakAtomic<int>   currentBeat  = 0;
	};

	Shared* shared = nullptr;
};
} // namespace giada::m::model

#endif
