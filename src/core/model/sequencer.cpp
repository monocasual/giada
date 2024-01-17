/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "core/model/sequencer.h"
#include "core/const.h"
#include "utils/time.h"

namespace giada::m::model
{
bool Sequencer::isActive() const
{
	return status == SeqStatus::RUNNING || status == SeqStatus::WAITING;
}

/* -------------------------------------------------------------------------- */

bool Sequencer::canQuantize() const
{
	return quantize > 0 && status == SeqStatus::RUNNING;
}

/* -------------------------------------------------------------------------- */

bool Sequencer::isRunning() const
{
	return status == SeqStatus::RUNNING;
}

/* -------------------------------------------------------------------------- */

bool Sequencer::a_isOnBar() const
{
	const int currentFrame = shared->currentFrame.load();

	if (status == SeqStatus::WAITING || currentFrame == 0)
		return false;
	return currentFrame % framesInBar == 0;
}

/* -------------------------------------------------------------------------- */

bool Sequencer::a_isOnBeat() const
{
	return shared->currentFrame.load() % framesInBeat == 0;
}

/* -------------------------------------------------------------------------- */

bool Sequencer::a_isOnFirstBeat() const
{
	return shared->currentFrame.load() == 0;
}

/* -------------------------------------------------------------------------- */

Frame Sequencer::a_getCurrentFrame() const { return shared->currentFrame.load(); }
Frame Sequencer::a_getCurrentBeat() const { return shared->currentBeat.load(); }

/* -------------------------------------------------------------------------- */

float Sequencer::a_getCurrentSecond(int sampleRate) const
{
	return a_getCurrentFrame() / static_cast<float>(sampleRate);
}

/* -------------------------------------------------------------------------- */

int Sequencer::getMaxFramesInLoop(int sampleRate) const
{
	return (sampleRate * (60.0f / G_MIN_BPM)) * beats;
}

/* -------------------------------------------------------------------------- */

void Sequencer::a_setCurrentFrame(Frame f, int sampleRate) const
{
	shared->currentFrame.store(f);
	shared->currentBeat.store(f == 0 ? 0 : u::time::frameToBeat(f, sampleRate, bpm));
}

void Sequencer::a_setCurrentBeat(int b, int sampleRate) const
{
	const Frame currentFrame = u::time::beatToFrame(b, sampleRate, bpm);

	shared->currentFrame.store(currentFrame);
	shared->currentBeat.store(b);
}
} // namespace giada::m::model
