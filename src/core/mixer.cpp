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

#include "src/core/mixer.h"
#include "src/core/const.h"
#include "src/deps/mcl-utils/src/math.hpp"
#include "src/utils/log.h"

namespace math = mcl::utils::math;

namespace giada::m
{
namespace
{
/* CH_LEFT, CH_RIGHT
Channels identifiers. */

constexpr int CH_LEFT  = 0;
constexpr int CH_RIGHT = 1;
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Mixer::Mixer(model::RealtimeModel& m)
: onSignalTresholdReached(nullptr)
, onEndOfRecording(nullptr)
, m_model(m)
, m_signalCbFired(false)
, m_endOfRecCbFired(false)
{
}

/* -------------------------------------------------------------------------- */

void Mixer::reset(int maxFramesInLoop, int framesInBuffer)
{
	m_model.writeAssets(model::SwapType::SOFT, [maxFramesInLoop, framesInBuffer](model::Assets& assets)
	{
		assets.mixerRendering = std::make_shared<model::Mixer::Rendering>(maxFramesInLoop, framesInBuffer);
	});

	m_model.flush();
}

/* -------------------------------------------------------------------------- */

void Mixer::enable()
{
	m_model.getParameters().mixer.setActive(true);
	u::log::print("[mixer::enable] enabled\n");
}

void Mixer::disable()
{
	m_model.getParameters().mixer.setActive(false);
	u::log::print("[mixer::disable] disabled\n");
}

/* -------------------------------------------------------------------------- */

void Mixer::handleAudioInput(const mcl::AudioBuffer& input, const model::ModelView& view) const
{
	const model::Document& doc       = view.getDocument();
	const auto&            rendering = *view.getAssets().mixerRendering;
	const model::Channel&  masterIn  = doc.tracks.getChannel(MASTER_IN_CHANNEL_ID);

	const Peak peak = prepareAudioInput(input, rendering, masterIn.volume);

	if (!input.isAllocd())
		return;

	const bool sequencerIsActive = doc.sequencer.isActive();
	checkInputSignal(peak, doc.kernelAudio.recTriggerLevel, sequencerIsActive);

	if (sequencerIsActive && doc.mixer.isRecordingInput)
	{
		const bool allowsOverdub = doc.mixer.inputRecMode == InputRecMode::RIGID;
		const int  sampleRate    = doc.kernelAudio.samplerate;
		const int  maxFrames     = doc.mixer.inputRecMode == InputRecMode::FREE
		                               ? doc.sequencer.getMaxFramesInLoop(sampleRate)
		                               : doc.sequencer.getFramesInLoop(sampleRate);

		recordAudioInput(rendering, input.countFrames(), maxFrames, allowsOverdub);
	}
}

/* -------------------------------------------------------------------------- */

Peak Mixer::prepareAudioInput(
    const mcl::AudioBuffer&        hardwareInput,
    const model::Mixer::Rendering& rendering,
    float                          masterInputGain) const
{
	rendering.prepareInputBufferFrom(hardwareInput, masterInputGain);

	const Peak peak = makePeak(rendering.getInBuffer());
	m_model.getParameters().mixer.setPeakIn(peak);
	return peak;
}

/* -------------------------------------------------------------------------- */

void Mixer::checkInputSignal(Peak peak, float threshold, bool sequencerIsActive) const
{
	if (sequencerIsActive && !m_signalCbFired && thresholdReached(peak, threshold))
	{
		m_signalCbFired = true;
		onSignalTresholdReached();
		G_DEBUG("Signal > threshold!", );
	}
}

/* -------------------------------------------------------------------------- */

void Mixer::recordAudioInput(
    const model::Mixer::Rendering& rendering,
    int                            inputFrames,
    int                            maxFrames,
    bool                           allowsOverdub) const
{
	const auto& params  = m_model.getParameters().mixer;
	const Frame tracker = params.getInputTracker();

	assert(maxFrames > 0 && maxFrames <= rendering.getRecBuffer().countFrames());
	assert(onEndOfRecording != nullptr);

	if (tracker >= maxFrames && !allowsOverdub && !m_endOfRecCbFired)
	{
		onEndOfRecording();
		m_endOfRecCbFired = true;
		params.setInputTracker(0);
		return;
	}

	rendering.sumInputToRecBuffer(tracker % maxFrames);
	params.setInputTracker(tracker + inputFrames);
}

/* -------------------------------------------------------------------------- */

void Mixer::allocRecBuffer(Frame size)
{
	m_model.getCurrentAssets()->mixerRendering->resizeRecBuffer(size);
}

void Mixer::clearRecBuffer()
{
	return m_model.getCurrentAssets()->mixerRendering->clearRecBuffer();
}

const mcl::AudioBuffer& Mixer::getRecBuffer() const
{
	return m_model.getCurrentAssets()->mixerRendering->getRecBuffer();
}

/* -------------------------------------------------------------------------- */

void Mixer::updateSoloCount(bool hasSolos)
{
	m_model.writeDocument(model::SwapType::NONE, [hasSolos](model::Document& doc)
	{
		doc.mixer.hasSolos = hasSolos;
	});
}

/* -------------------------------------------------------------------------- */

void Mixer::setInToOut(bool v)
{
	m_model.writeDocument(model::SwapType::NONE, [v](model::Document& doc)
	{
		doc.mixer.inToOut = v;
	});
}

/* -------------------------------------------------------------------------- */

void Mixer::setRecTriggerMode(RecTriggerMode m)
{
	m_model.writeDocument(model::SwapType::NONE, [m](model::Document& doc)
	{
		doc.mixer.recTriggerMode = m;
	});
}

void Mixer::setInputRecMode(InputRecMode m)
{
	m_model.writeDocument(model::SwapType::NONE, [m](model::Document& doc)
	{
		doc.mixer.inputRecMode = m;
	});
}

/* -------------------------------------------------------------------------- */

void Mixer::startInputRec(Frame from)
{
	m_model.getParameters().mixer.setInputTracker(from);
	m_model.writeDocument(model::SwapType::SOFT, [](model::Document& doc)
	{
		doc.mixer.isRecordingInput = true;
	});
}

Frame Mixer::stopInputRec()
{
	const Frame ret = m_model.getParameters().mixer.getInputTracker();
	m_model.getParameters().mixer.setInputTracker(0);
	m_model.writeDocument(model::SwapType::SOFT, [](model::Document& doc)
	{
		doc.mixer.isRecordingInput = false;
	});
	m_signalCbFired   = false;
	m_endOfRecCbFired = false;
	return ret;
}

/* -------------------------------------------------------------------------- */

void Mixer::startActionRec()
{
	m_model.writeDocument(model::SwapType::NONE, [](model::Document& doc)
	{
		doc.mixer.isRecordingActions = true;
	});
}

void Mixer::stopActionRec()
{
	m_model.writeDocument(model::SwapType::NONE, [](model::Document& doc)
	{
		doc.mixer.isRecordingActions = false;
	});
}

/* -------------------------------------------------------------------------- */

Peak Mixer::getPeakOut() const { return m_model.getParameters().mixer.getPeakOut(); }
Peak Mixer::getPeakIn() const { return m_model.getParameters().mixer.getPeakIn(); }

/* -------------------------------------------------------------------------- */

bool Mixer::getInToOut() const { return m_model.getCurrentDocument()->mixer.inToOut; }

/* -------------------------------------------------------------------------- */

Mixer::RecordInfo Mixer::getRecordInfo() const
{
	return {
	    m_model.getParameters().mixer.getInputTracker(),
	    m_model.getCurrentAssets()->mixerRendering->getRecBuffer().countFrames()};
}

/* -------------------------------------------------------------------------- */

bool Mixer::isRecordingActions() const
{
	return m_model.getCurrentDocument()->mixer.isRecordingActions;
}

bool Mixer::isRecordingInput() const
{
	return m_model.getCurrentDocument()->mixer.isRecordingInput;
}

/* -------------------------------------------------------------------------- */

RecTriggerMode Mixer::getRecTriggerMode() const
{
	return m_model.getCurrentDocument()->mixer.recTriggerMode;
}

InputRecMode Mixer::getInputRecMode() const
{
	return m_model.getCurrentDocument()->mixer.inputRecMode;
}

/* -------------------------------------------------------------------------- */

bool Mixer::thresholdReached(Peak p, float threshold) const
{
	return math::linearToDB(p.left) > threshold ||
	       math::linearToDB(p.right) > threshold;
}

/* -------------------------------------------------------------------------- */

Peak Mixer::makePeak(const mcl::AudioBuffer& b) const
{
	if (!b.isAllocd())
		return {0.0f, 0.0f};
	return {b.getPeak(CH_LEFT), b.getPeak(b.countChannels() == 1 ? CH_LEFT : CH_RIGHT)};
}

/* -------------------------------------------------------------------------- */

void Mixer::limit(mcl::AudioBuffer& outBuf) const
{
	for (int i = 0; i < outBuf.countFrames(); i++)
		for (int j = 0; j < outBuf.countChannels(); j++)
			outBuf.at(i, j) = std::max(-1.0f, std::min(outBuf.at(i, j), 1.0f));
}

/* -------------------------------------------------------------------------- */

void Mixer::finalizeOutput(
    const mcl::AudioBuffer& mixerIn,
    mcl::AudioBuffer&       hardwareOut,
    bool                    inToOut,
    bool                    shouldLimit,
    float                   vol) const
{
	if (inToOut)
		hardwareOut.sumAll(mixerIn, vol);
	else
		hardwareOut.applyGain(vol);

	if (shouldLimit)
		limit(hardwareOut);
}

/* -------------------------------------------------------------------------- */

void Mixer::updateOutputPeak(
    const model::Mixer::Parameters& params,
    const mcl::AudioBuffer&         buf) const
{
	params.setPeakOut({buf.getPeak(CH_LEFT), buf.getPeak(CH_RIGHT)});
}
} // namespace giada::m
