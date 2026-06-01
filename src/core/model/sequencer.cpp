/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/core/model/sequencer.h"
#include "src/core/const.h"
#include "src/utils/time.h"

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

Scene Sequencer::a_getCurrentScene() const
{
	return Scene{shared->currentSceneIndex.load()};
}

Scene Sequencer::a_getNextScene() const
{
	return Scene{shared->nextSceneIndex.load()};
}

/* -------------------------------------------------------------------------- */

SceneStatus Sequencer::a_getSceneStatus() const
{
	return shared->sceneStatus.load();
}

/* -------------------------------------------------------------------------- */

int Sequencer::getMaxFramesInLoop(int sampleRate) const
{
	return (sampleRate * (60.0f / G_MIN_BPM)) * m_timeSignature.beats;
}

/* -------------------------------------------------------------------------- */

void Sequencer::a_setCurrentFrame(Frame f, int sampleRate) const
{
	shared->currentFrame.store(f);
	shared->currentBeat.store(f == 0 ? 0 : u::time::frameToBeat(f, sampleRate, m_bpm));
}

void Sequencer::a_setCurrentBeat(int b, int sampleRate) const
{
	const Frame currentFrame = u::time::beatToFrame(b, sampleRate, m_bpm);

	shared->currentFrame.store(currentFrame);
	shared->currentBeat.store(b);
}

/* -------------------------------------------------------------------------- */

void Sequencer::a_setCurrentScene(Scene scene) const
{
	shared->currentSceneIndex.store(scene.getIndex());
}

void Sequencer::a_setNextScene(Scene scene) const
{
	shared->nextSceneIndex.store(scene.getIndex());
}

/* -------------------------------------------------------------------------- */

void Sequencer::a_setSceneStatus(SceneStatus s) const
{
	shared->sceneStatus.store(s);
}

/* -------------------------------------------------------------------------- */

float         Sequencer::getBpm() const { return m_bpm; }
TimeSignature Sequencer::getTimeSignature() const { return m_timeSignature; }
Tick          Sequencer::getTicksInBeat() const { return G_PPQ; }
Tick          Sequencer::getTicksInBar() const { return getTicksInLoop() / m_timeSignature.bars; }
Tick          Sequencer::getTicksInLoop() const { return G_PPQ * m_timeSignature.beats; }
Tick          Sequencer::getTicksInSeq() const { return G_PPQ * G_MAX_BEATS; }

/* -------------------------------------------------------------------------- */

void Sequencer::reset()
{
	m_bpm           = G_DEFAULT_BPM;
	m_timeSignature = {};
	quantize        = G_DEFAULT_QUANTIZE;
}

void Sequencer::recomputeFrames(int sampleRate)
{
	framesInBeat = u::time::beatToFrame(1, sampleRate, m_bpm);
	framesInLoop = framesInBeat * m_timeSignature.beats;
	framesInBar  = framesInLoop / (float)m_timeSignature.bars;
	framesInSeq  = framesInBeat * G_MAX_BEATS;
}

/* -------------------------------------------------------------------------- */

void Sequencer::setBpm(float v)
{
	m_bpm = std::clamp(v, G_MIN_BPM, G_MAX_BPM);
}

void Sequencer::setTimeSignature(TimeSignature t)
{
	t.beats = std::clamp(t.beats, 1, G_MAX_BEATS);
	t.bars  = std::clamp(t.bars, 1, t.beats); // Bars cannot be greater than beats

	m_timeSignature = t;
}
} // namespace giada::m::model
