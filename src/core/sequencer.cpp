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

#include "src/core/sequencer.h"
#include "src/core/actions/ActionManager.h"
#include "src/core/jackTransport.h"
#include "src/core/kernelAudio.h"
#include "src/core/metronome.h"
#include "src/core/midiSynchronizer.h"
#include "src/core/model/actions.h"
#include "src/core/model/model.h"
#include "src/core/model/sequencer.h"
#include "src/core/model/types.h"
#include "src/deps/mcl-utils/src/math.hpp"
#include "src/utils/log.h"
#include "src/utils/time.h"

namespace giada::m
{
namespace
{
/* AudioBlock
A pair of ranges that describe the current audio block to render from the Sequencer
perspective, taking the wraparound at first beat into account. Head is always
valid, tail can be valid if there is a wraparound in the current block. The
'onEachFrame' helper method allows to process both ranges in one shot. */

struct AudioBlock
{
	FrameRange head;
	FrameRange tail;
	Frame      nextFrame;

	AudioBlock(Frame start, Frame end, int framesInLoop)
	: head(start, std::min(end, framesInLoop))
	, nextFrame(end % framesInLoop)
	{
		if (end > framesInLoop)
			tail = {0, nextFrame};

		assert(end - start == head.getLength() + tail.getLength());
	}

	bool contains(Frame f) const
	{
		return head.contains(f) || tail.contains(f);
	}

	template <typename F>
	void onEachFrame(F&& fn) const
	{
		Frame local = 0;
		for (Frame global = head.getA(); global < head.getB(); global++)
			fn(global, local++);
		if (tail.isValid())
			for (Frame global = tail.getA(); global < tail.getB(); global++)
				fn(global, local++);
	}
};

/* -------------------------------------------------------------------------- */

/* ActionsBlock
Same concept of Block above, but here it's just a small helper to loop over the
entire block, contiguously. */

struct ActionsBlock
{
	std::span<const model::Action> head;
	std::span<const model::Action> tail;

	template <typename F>
	void onEachAction(F&& fn) const
	{
		for (const model::Action& a : head)
			fn(a);
		for (const model::Action& a : tail)
			fn(a);
	}
};
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Sequencer::Sequencer(model::Model& m, MidiSynchronizer& s, JackTransport& j)
: onAboutStart(nullptr)
, onAboutStop(nullptr)
, onSceneChanged(nullptr)
, m_model(m)
, m_midiSynchronizer(s)
, m_jackTransport(j)
, m_quantizerStep(1)
, m_currentSampleRate(0)
{
	m_eventBuffer.reserve(G_MAX_SEQUENCER_EVENTS);
}
/* -------------------------------------------------------------------------- */

bool          Sequencer::canQuantize() const { return m_model.read().getDocument().sequencer.canQuantize(); }
bool          Sequencer::isRunning() const { return m_model.read().getDocument().sequencer.isRunning(); }
float         Sequencer::getBpm() const { return m_model.read().getDocument().sequencer.getBpm(); }
TimeSignature Sequencer::getTimeSignature() const { return m_model.read().getDocument().sequencer.getTimeSignature(); }
int           Sequencer::getCurrentBeat() const { return m_model.read().getParameters().sequencer.getCurrentBeat(); }
Frame         Sequencer::getCurrentFrame() const { return m_model.read().getParameters().sequencer.getCurrentFrame(); }
Frame         Sequencer::getFramesInBar() const { return u::time::tickToFrame(m_model.read().getDocument().sequencer.getTicksInBar(), m_currentSampleRate, getBpm()); }
Frame         Sequencer::getFramesInBeat() const { return u::time::tickToFrame(m_model.read().getDocument().sequencer.getTicksInBeat(), m_currentSampleRate, getBpm()); }
Frame         Sequencer::getFramesInLoop() const { return u::time::tickToFrame(m_model.read().getDocument().sequencer.getTicksInLoop(), m_currentSampleRate, getBpm()); }
Frame         Sequencer::getFramesInSeq() const { return u::time::tickToFrame(m_model.read().getDocument().sequencer.getTicksInSeq(), m_currentSampleRate, getBpm()); }
Tick          Sequencer::getTicksInBar() const { return m_model.read().getDocument().sequencer.getTicksInBar(); }
Tick          Sequencer::getTicksInBeat() const { return m_model.read().getDocument().sequencer.getTicksInBeat(); }
Tick          Sequencer::getTicksInLoop() const { return m_model.read().getDocument().sequencer.getTicksInLoop(); }
Tick          Sequencer::getTicksInSeq() const { return m_model.read().getDocument().sequencer.getTicksInSeq(); }
int           Sequencer::getQuantizerValue() const { return m_model.read().getDocument().sequencer.quantize; }
int           Sequencer::getQuantizerStep() const { return m_quantizerStep; }
SeqStatus     Sequencer::getStatus() const { return m_model.read().getDocument().sequencer.status; }
int           Sequencer::getMaxFramesInLoop(int sampleRate) const { return m_model.read().getDocument().sequencer.getMaxFramesInLoop(sampleRate); }
Scene         Sequencer::getCurrentScene() const { return m_model.read().getParameters().sequencer.getCurrentScene(); }
Scene         Sequencer::getNextScene() const { return m_model.read().getParameters().sequencer.getNextScene(); }
SceneStatus   Sequencer::getSceneStatus() const { return m_model.read().getParameters().sequencer.getSceneStatus(); }

/* -------------------------------------------------------------------------- */

Tick Sequencer::getCurrentTick() const { return m_model.read().getParameters().sequencer.getCurrentTick(m_currentSampleRate, getBpm()); }
Tick Sequencer::getCurrentTickQuantized() const { return quantize(getCurrentTick()); }

/* -------------------------------------------------------------------------- */

float Sequencer::calcBpmFromRec(Frame recordedFrames) const
{
	return (60.0f * getTimeSignature().beats) / (recordedFrames / static_cast<float>(m_currentSampleRate));
}

/* -------------------------------------------------------------------------- */

Tick Sequencer::quantize(Tick t) const
{
	namespace math = mcl::utils::math;

	if (!canQuantize())
		return t;
	return Tick{math::quantize(t.value(), Tick::Value{m_quantizerStep})} % getTicksInLoop(); // No overflow
}

/* -------------------------------------------------------------------------- */

void Sequencer::reset(int sampleRate)
{
	m_currentSampleRate = sampleRate;

	m_model.write(model::SwapType::SOFT, [](model::DocumentEdit& edit)
	{
		edit.get().sequencer.reset();
	});

	m_model.flush();

	rewind();
}

/* -------------------------------------------------------------------------- */

void Sequencer::setSampleRate(int sampleRate)
{
	m_currentSampleRate = sampleRate;
}

/* -------------------------------------------------------------------------- */

const Sequencer::EventBuffer& Sequencer::advance(
    const model::Sequencer&             sequencer,
    const model::Sequencer::Parameters& params,
    const model::Actions&               actions,
    Frame                               bufferSize) const
{
	m_eventBuffer.clear();

	const Frame start        = params.getCurrentFrame();
	const Frame end          = start + bufferSize;
	const float bpm          = sequencer.getBpm();
	const Frame framesInLoop = u::time::tickToFrame(sequencer.getTicksInLoop(), m_currentSampleRate, bpm);
	const Frame framesInBar  = u::time::tickToFrame(sequencer.getTicksInBar(), m_currentSampleRate, bpm);
	const Frame framesInBeat = u::time::tickToFrame(sequencer.getTicksInBeat(), m_currentSampleRate, bpm);

	const Scene currentScene = params.getCurrentScene();
	const Scene nextScene    = params.getNextScene();
	bool        sceneChanged = false;

	/* Process events in the current block. */

	const AudioBlock   audioBlock(start, end, framesInLoop);
	const ActionsBlock actionsBlock{
	    actions.getActionsInTickRange(u::time::frameRangeToTickRange(audioBlock.head, m_currentSampleRate, bpm)),
	    actions.getActionsInTickRange(u::time::frameRangeToTickRange(audioBlock.tail, m_currentSampleRate, bpm))};

	audioBlock.onEachFrame([&, this](Frame global, Frame local)
	{
		if (global == 0)
		{
			m_eventBuffer.push_back({EventType::FIRST_BEAT, global, local});
			m_metronome.trigger(Metronome::Click::BEAT, local);
			if (currentScene != nextScene)
			{
				assert(onSceneChanged != nullptr);
				params.setCurrentScene(nextScene);
				params.setSceneStatus(SceneStatus::IDLE);
				sceneChanged = true;
				onSceneChanged(); // Can't directly swap model here, this is real-time stuff
			}
		}
		else if (global % framesInBar == 0)
		{
			m_eventBuffer.push_back({EventType::BAR, global, local});
			m_metronome.trigger(Metronome::Click::BAR, local);
		}
		else if (global % framesInBeat == 0)
		{
			m_metronome.trigger(Metronome::Click::BEAT, local);
		}
	});

	/* Push actions from the current block into the event buffer. Extra care is
	needed if the scene has changed in this block: we need to process actions
	that belong to the next scene, not the current one (which is the old one). */

	actionsBlock.onEachAction([&](const model::Action& action)
	{
		/* An action may fall just outside the current block because the frame-to-tick
		conversion above (AudioBlock -> ActionsBlock) is conservative. Double-check
		here that we do not include actions that are not actually in the block. */

		const Frame frame = u::time::tickToFrame(action.tick, m_currentSampleRate, bpm);
		if (!audioBlock.contains(frame))
			return;

		/* Adjusting local offset requires more tweaking if the action belongs to
		the tail block: it means the sequencer has wrapped around the loop and
		we are fetching actions from the beginning. */

		bool        inTail = audioBlock.tail.isValid() && audioBlock.tail.contains(frame);
		const Frame offset = inTail ? audioBlock.head.getLength() : -audioBlock.head.getA();
		const Frame local  = frame + offset;

		m_eventBuffer.push_back({EventType::ACTIONS, 0, local, action, sceneChanged ? nextScene : currentScene});
	});

	/* Advance after the event parsing. */

	params.setCurrentFrame(audioBlock.nextFrame, m_currentSampleRate, bpm);

	return m_eventBuffer;
}

/* -------------------------------------------------------------------------- */

void Sequencer::render(mcl::AudioBuffer& outBuf) const
{
	m_metronome.render(outBuf);
}

/* -------------------------------------------------------------------------- */

void Sequencer::rawStart()
{
	assert(onAboutStart != nullptr);

	const SeqStatus status = getStatus();

	onAboutStart(status);

	switch (status)
	{
	case SeqStatus::STOPPED:
		setStatus(SeqStatus::RUNNING);
		break;
	case SeqStatus::WAITING:
		setStatus(SeqStatus::RUNNING);
		break;
	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void Sequencer::rawStop()
{
	assert(onAboutStop != nullptr);

	onAboutStop();
	setStatus(SeqStatus::STOPPED);
}

/* -------------------------------------------------------------------------- */

void Sequencer::rawRewind(Frame delta)
{
	rewindForced();
	m_eventBuffer.push_back({EventType::REWIND, 0, delta});
}

/* -------------------------------------------------------------------------- */

void Sequencer::rewindForced()
{
	// No need for sampleRate and bpm values, everything it's just 0
	m_model.read().getParameters().sequencer.setCurrentFrame(0, /*sampleRate=*/0, /*bpm=*/0);
}

/* -------------------------------------------------------------------------- */

void Sequencer::rewind()
{
	if (canQuantize())
	{
		// TODO: Restore quantized rewind when the quantizer is reworked.
		return;
	}
	else if (!m_jackTransport.setPosition(0))
		rawRewind(0);
}

/* -------------------------------------------------------------------------- */

void Sequencer::start()
{
	if (!m_jackTransport.start())
		rawStart();
}

/* -------------------------------------------------------------------------- */

void Sequencer::stop()
{
	if (!m_jackTransport.stop())
		rawStop();

	const auto  view   = m_model.read();
	const auto& params = view.getParameters().sequencer;

	if (params.getSceneStatus() == SceneStatus::CHANGING)
	{
		params.setSceneStatus(SceneStatus::IDLE);
		params.setNextScene(params.getCurrentScene());
	}
}

/* -------------------------------------------------------------------------- */

bool Sequencer::isMetronomeOn() const { return m_model.read().getDocument().sequencer.metronome; }

void Sequencer::toggleMetronome() { setMetronome(!isMetronomeOn()); }

void Sequencer::setMetronome(bool v)
{
	m_model.write(model::SwapType::SOFT, [v](model::DocumentEdit& edit)
	{
		edit.get().sequencer.metronome = v;
	});
}

/* -------------------------------------------------------------------------- */

void Sequencer::setBpm(float b)
{
	b = std::clamp(b, G_MIN_BPM, G_MAX_BPM);

	/* If JACK is being used, let it handle the bpm change. */
	if (!m_jackTransport.setBpm(b))
		rawSetBpm(b);
}

/* -------------------------------------------------------------------------- */

void Sequencer::rawSetBpm(float v)
{
	m_model.write(model::SwapType::HARD, [v](model::DocumentEdit& edit)
	{
		edit.get().sequencer.setBpm(v);
	});

	u::log::print("[sequencer::rawSetBpm] Bpm changed to {}\n", v);
}

/* -------------------------------------------------------------------------- */

void Sequencer::setTimeSignature(TimeSignature t)
{
	m_model.write(model::SwapType::HARD, [t](model::DocumentEdit& edit)
	{
		edit.get().sequencer.setTimeSignature(t);
	});
}

/* -------------------------------------------------------------------------- */

void Sequencer::rawGoToBeat(int beat, int sampleRate)
{
	m_model.read().getParameters().sequencer.setCurrentBeat(beat, sampleRate, getBpm());
}

/* -------------------------------------------------------------------------- */

void Sequencer::setQuantize(int q)
{
	m_model.write(model::SwapType::HARD, [q](model::DocumentEdit& edit)
	{
		edit.get().sequencer.quantize = q;
	});
}

/* -------------------------------------------------------------------------- */

void Sequencer::setStatus(SeqStatus s)
{
	m_model.write(model::SwapType::SOFT, [s](model::DocumentEdit& edit)
	{
		edit.get().sequencer.status = s;
	});

	/* Additional things to do when the status changes. */

	switch (s)
	{
	case SeqStatus::WAITING:
		rewind();
		m_midiSynchronizer.sendRewind();
		break;
	case SeqStatus::STOPPED:
		m_midiSynchronizer.sendStop();
		break;
	case SeqStatus::RUNNING:
		m_midiSynchronizer.sendStart();
		break;
	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void Sequencer::goToBeat(int beat, int sampleRate)
{
	const float bpm   = getBpm();
	const Frame frame = u::time::beatToFrame(beat, sampleRate, bpm);
	if (!m_jackTransport.setPosition(frame))
		rawGoToBeat(beat, sampleRate);
}

/* -------------------------------------------------------------------------- */

void Sequencer::setScene(Scene scene, bool forced)
{
	assert(scene.isValid());

	// TODO - we HARD swap only to refresh the whole UI on scene change. Maybe
	// there is a better way.

	m_model.write(model::SwapType::HARD, [scene, forced](model::DocumentEdit& edit)
	{
		const model::Document& doc    = edit.get();
		const auto&            params = edit.view().getParameters().sequencer;

		const Scene currentScene = params.getCurrentScene();
		const bool  sameScene    = scene == currentScene;

		if (sameScene)
		{
			if (!doc.sequencer.isRunning())
				return;
			params.setNextScene(scene);
			params.setSceneStatus(SceneStatus::IDLE);
			return;
		}
		if (!doc.sequencer.isRunning() || forced)
		{
			params.setCurrentScene(scene);
			params.setSceneStatus(SceneStatus::IDLE);
		}
		else
			params.setSceneStatus(SceneStatus::CHANGING);
		params.setNextScene(scene);
	});
}

/* -------------------------------------------------------------------------- */

#ifdef WITH_AUDIO_JACK

void Sequencer::jack_start()
{
	rawStart();
}

void Sequencer::jack_stop() { rawStop(); }
void Sequencer::jack_rewind() { rawRewind(0); }
void Sequencer::jack_setBpm(float b) { rawSetBpm(b); }

#endif
} // namespace giada::m
