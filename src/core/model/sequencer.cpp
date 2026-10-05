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
bool Sequencer::Parameters::isOnBar(int sampleRate, Tick ticksInBar, SeqStatus status, float bpm) const
{
	const Tick currentTick = getCurrentTick(sampleRate, bpm);
	if (status == SeqStatus::WAITING || currentTick == Tick{0})
		return false;
	return currentTick % ticksInBar == Tick{0};
}

/* -------------------------------------------------------------------------- */

bool Sequencer::Parameters::isOnBeat(int sampleRate, Tick ticksInBeat, float bpm) const
{
	return getCurrentTick(sampleRate, bpm) % ticksInBeat == Tick{0};
}

/* -------------------------------------------------------------------------- */

bool Sequencer::Parameters::isOnFirstBeat() const
{
	return m_currentFrame.load() == 0;
}

/* -------------------------------------------------------------------------- */

Frame Sequencer::Parameters::getCurrentFrame() const
{
	return m_currentFrame.load();
}

/* -------------------------------------------------------------------------- */

Tick Sequencer::Parameters::getCurrentTick(int sampleRate, float bpm) const
{
	return u::time::frameToTickFloor(getCurrentFrame(), sampleRate, bpm);
}

/* -------------------------------------------------------------------------- */

Frame Sequencer::Parameters::getCurrentBeat() const
{
	return m_currentBeat.load();
}

/* -------------------------------------------------------------------------- */

float Sequencer::Parameters::getCurrentSecond(int sampleRate) const
{
	return getCurrentFrame() / static_cast<float>(sampleRate);
}

/* -------------------------------------------------------------------------- */

double Sequencer::Parameters::getCurrentQuarterNotePosition(int sampleRate, float bpm) const
{
	return u::time::frameToQuarterNotes(getCurrentFrame(), sampleRate, bpm);
}

/* -------------------------------------------------------------------------- */

Scene Sequencer::Parameters::getCurrentScene() const
{
	return Scene{m_currentSceneIndex.load()};
}

/* -------------------------------------------------------------------------- */

Scene Sequencer::Parameters::getNextScene() const
{
	return Scene{m_nextSceneIndex.load()};
}

/* -------------------------------------------------------------------------- */

SceneStatus Sequencer::Parameters::getSceneStatus() const
{
	return m_sceneStatus.load();
}

/* -------------------------------------------------------------------------- */

void Sequencer::Parameters::setCurrentFrame(Frame f, int sampleRate, float bpm) const
{
	m_currentFrame.store(f);
	m_currentBeat.store(f == 0 ? 0 : u::time::frameToBeat(f, sampleRate, bpm));
}

/* -------------------------------------------------------------------------- */

void Sequencer::Parameters::setCurrentBeat(int b, int sampleRate, float bpm) const
{
	m_currentFrame.store(u::time::beatToFrame(b, sampleRate, bpm));
	m_currentBeat.store(b);
}

/* -------------------------------------------------------------------------- */

void Sequencer::Parameters::setCurrentScene(Scene scene) const
{
	m_currentSceneIndex.store(scene.getIndex());
}

/* -------------------------------------------------------------------------- */

void Sequencer::Parameters::setNextScene(Scene scene) const
{
	m_nextSceneIndex.store(scene.getIndex());
}

/* -------------------------------------------------------------------------- */

void Sequencer::Parameters::setSceneStatus(SceneStatus s) const
{
	m_sceneStatus.store(s);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

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

int Sequencer::getFramesInLoop(int sampleRate) const
{
	return u::time::tickToFrame(getTicksInLoop(), sampleRate, m_bpm);
}

/* -------------------------------------------------------------------------- */

int Sequencer::getMaxFramesInLoop(int sampleRate) const
{
	return (sampleRate * (60.0f / G_MIN_BPM)) * m_timeSignature.beats;
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
