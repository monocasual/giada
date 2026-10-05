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

#ifndef G_MODEL_SEQUENCER_H
#define G_MODEL_SEQUENCER_H

#include "src/core/const.h"
#include "src/core/types.h"
#include "src/core/weakAtomic.h"
#include "src/deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include "src/scene.h"

namespace giada::m::model
{
class Sequencer
{
public:
	class Parameters
	{
	public:
		bool        isOnBar(int sampleRate, Tick ticksInBars, SeqStatus, float bpm) const;
		bool        isOnBeat(int sampleRate, Tick ticksInBeat, float bpm) const;
		bool        isOnFirstBeat() const;
		Frame       getCurrentFrame() const;
		Tick        getCurrentTick(int sampleRate, float bpm) const;
		Frame       getCurrentBeat() const;
		float       getCurrentSecond(int sampleRate) const;
		double      getCurrentQuarterNotePosition(int sampleRate, float bpm) const;
		Scene       getCurrentScene() const;
		Scene       getNextScene() const;
		SceneStatus getSceneStatus() const;

		void setCurrentFrame(Frame f, int sampleRate, float bpm) const;
		void setCurrentBeat(int b, int sampleRate, float bpm) const;
		void setCurrentScene(Scene) const;
		void setNextScene(Scene) const;
		void setSceneStatus(SceneStatus) const;

	private:
		mutable WeakAtomic<Frame>       m_currentFrame      = 0;
		mutable WeakAtomic<int>         m_currentBeat       = 0;
		mutable WeakAtomic<std::size_t> m_currentSceneIndex = 0;
		mutable WeakAtomic<std::size_t> m_nextSceneIndex    = 0;

		/* m_sceneStatus
		IDLE = current scene selected. CHANGING = a change of scene has been requested
		and will go back to IDLE at the next first beat. */

		mutable WeakAtomic<SceneStatus> m_sceneStatus = SceneStatus::IDLE;
	};

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

	/* getFramesInLoop
	Returns the number of frames in the current loop. */

	int getFramesInLoop(int sampleRate) const;

	/* getMaxFramesInLoop
	Returns how many frames the current loop length might contain at the slowest
	speed possible (G_MIN_BPM). */

	int getMaxFramesInLoop(int sampleRate) const;

	float         getBpm() const;
	TimeSignature getTimeSignature() const;
	Tick          getTicksInBeat() const;
	Tick          getTicksInBar() const;
	Tick          getTicksInLoop() const;
	Tick          getTicksInSeq() const;

	/* reset
	Resets beats, bars, bpm and quantize to default values. */

	void reset();

	void setBpm(float);
	void setTimeSignature(TimeSignature);

	SeqStatus status    = SeqStatus::STOPPED;
	int       quantize  = G_DEFAULT_QUANTIZE;
	bool      metronome = false;

private:
	float         m_bpm = G_DEFAULT_BPM;
	TimeSignature m_timeSignature;
};
} // namespace giada::m::model

#endif
