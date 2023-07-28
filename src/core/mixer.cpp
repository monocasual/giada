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

#include "core/mixer.h"
#include "core/const.h"
#include "core/model/model.h"
#include "utils/log.h"
#include "utils/math.h"

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

Mixer::Mixer(model::Model& m)
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
	/* Allocate working buffers. rec buffer has variable size: it depends on how
	many frames there are in the current loop. */

	m_model.get().mixer.getRecBuffer().alloc(maxFramesInLoop, G_MAX_IO_CHANS);
	m_model.get().mixer.getInBuffer().alloc(framesInBuffer, G_MAX_IO_CHANS);

	u::log::print("[mixer::reset] buffers ready - maxFramesInLoop={}, framesInBuffer={}\n",
	    maxFramesInLoop, framesInBuffer);
}

/* -------------------------------------------------------------------------- */

void Mixer::enable()
{
	m_model.get().mixer.a_setActive(true);
	u::log::print("[mixer::enable] enabled\n");
}

void Mixer::disable()
{
	m_model.get().mixer.a_setActive(false);
	while (m_model.isLocked())
		;
	u::log::print("[mixer::disable] disabled\n");
}

/* -------------------------------------------------------------------------- */

void Mixer::allocRecBuffer(int frames)
{
	m_model.get().mixer.getRecBuffer().alloc(frames, G_MAX_IO_CHANS);
}

void Mixer::clearRecBuffer()
{
	m_model.get().mixer.getRecBuffer().clear();
}

const mcl::AudioBuffer& Mixer::getRecBuffer()
{
	return m_model.get().mixer.getRecBuffer();
}

/* -------------------------------------------------------------------------- */

void Mixer::updateSoloCount(bool hasSolos)
{
	m_model.get().mixer.hasSolos = hasSolos;
	m_model.swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

void Mixer::setInToOut(bool v)
{
	m_model.get().mixer.inToOut = v;
	m_model.swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

void Mixer::setRecTriggerMode(RecTriggerMode m)
{
	m_model.get().mixer.recTriggerMode = m;
	m_model.swap(model::SwapType::NONE);
}

void Mixer::setInputRecMode(InputRecMode m)
{
	m_model.get().mixer.inputRecMode = m;
	m_model.swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

void Mixer::render(mcl::AudioBuffer& out, const mcl::AudioBuffer& in, const model::Layout& layout_RT, int maxFramesToRec) const
{
	const model::Mixer&       mixer       = layout_RT.mixer;
	const model::Sequencer&   sequencer   = layout_RT.sequencer;
	const model::Channels&    channels    = layout_RT.channels;
	const model::KernelAudio& kernelAudio = layout_RT.kernelAudio;

	const Channel& masterInCh = channels.get(Mixer::MASTER_IN_CHANNEL_ID);

	const bool  hasInput        = in.isAllocd();
	const bool  seqIsActive     = sequencer.isActive();
	const bool  shouldLineInRec = seqIsActive && mixer.isRecordingInput && hasInput;
	const float recTriggerLevel = kernelAudio.recTriggerLevel;
	const bool  allowsOverdub   = mixer.inputRecMode == InputRecMode::RIGID;

	mixer.getInBuffer().clear();

	/* Reset peak computation. */

	mixer.a_setPeakOut({0.0f, 0.0f});
	mixer.a_setPeakIn({0.0f, 0.0f});

	if (hasInput)
		processLineIn(mixer, in, masterInCh.volume, recTriggerLevel, seqIsActive);

	if (shouldLineInRec)
	{
		const Frame newTrackerPos = lineInRec(in, mixer.getRecBuffer(),
		    mixer.a_getInputTracker(), maxFramesToRec, masterInCh.volume,
		    allowsOverdub);
		mixer.a_setInputTracker(newTrackerPos);
	}
}

/* -------------------------------------------------------------------------- */

void Mixer::startInputRec(Frame from)
{
	m_model.get().mixer.a_setInputTracker(from);
	m_model.get().mixer.isRecordingInput = true;
	m_model.swap(model::SwapType::NONE);
}

Frame Mixer::stopInputRec()
{
	const Frame ret = m_model.get().mixer.a_getInputTracker();
	m_model.get().mixer.a_setInputTracker(0);
	m_model.get().mixer.isRecordingInput = false;
	m_model.swap(model::SwapType::NONE);
	m_signalCbFired   = false;
	m_endOfRecCbFired = false;
	return ret;
}

/* -------------------------------------------------------------------------- */

void Mixer::startActionRec()
{
	m_model.get().mixer.isRecordingActions = true;
	m_model.swap(model::SwapType::NONE);
}

void Mixer::stopActionRec()
{
	m_model.get().mixer.isRecordingActions = false;
	m_model.swap(model::SwapType::NONE);
}

/* -------------------------------------------------------------------------- */

Peak Mixer::getPeakOut() const { return m_model.get().mixer.a_getPeakOut(); }
Peak Mixer::getPeakIn() const { return m_model.get().mixer.a_getPeakIn(); }

/* -------------------------------------------------------------------------- */

bool Mixer::getInToOut() const { return m_model.get().mixer.inToOut; }

/* -------------------------------------------------------------------------- */

Mixer::RecordInfo Mixer::getRecordInfo() const
{
	return {
	    m_model.get().mixer.a_getInputTracker(),
	    m_model.get().mixer.getRecBuffer().countFrames()};
}

/* -------------------------------------------------------------------------- */

bool Mixer::isRecordingActions() const
{
	return m_model.get().mixer.isRecordingActions;
}

bool Mixer::isRecordingInput() const
{
	return m_model.get().mixer.isRecordingInput;
}

/* -------------------------------------------------------------------------- */

RecTriggerMode Mixer::getRecTriggerMode() const
{
	return m_model.get().mixer.recTriggerMode;
}

InputRecMode Mixer::getInputRecMode() const
{
	return m_model.get().mixer.inputRecMode;
}

/* -------------------------------------------------------------------------- */

bool Mixer::thresholdReached(Peak p, float threshold) const
{
	return u::math::linearToDB(p.left) > threshold ||
	       u::math::linearToDB(p.right) > threshold;
}

/* -------------------------------------------------------------------------- */

Peak Mixer::makePeak(const mcl::AudioBuffer& b) const
{
	if (!b.isAllocd())
		return {0.0f, 0.0f};
	return {b.getPeak(CH_LEFT), b.getPeak(b.countChannels() == 1 ? CH_LEFT : CH_RIGHT)};
}

/* -------------------------------------------------------------------------- */

int Mixer::lineInRec(const mcl::AudioBuffer& inBuf, mcl::AudioBuffer& recBuf, Frame inputTracker,
    int maxFrames, float inVol, bool allowsOverdub) const
{
	assert(maxFrames > 0 && maxFrames <= recBuf.countFrames());
	assert(onEndOfRecording != nullptr);

	if (inputTracker >= maxFrames && !allowsOverdub && !m_endOfRecCbFired)
	{
		onEndOfRecording();
		m_endOfRecCbFired = true;
		return 0;
	}

	const int   framesToCopy = -1; // copy everything
	const Frame srcOffset    = 0;
	const Frame destOffset   = inputTracker % maxFrames; // loop over at maxFrames

	recBuf.sum(inBuf, framesToCopy, srcOffset, destOffset, inVol);

	return inputTracker + inBuf.countFrames();
}

/* -------------------------------------------------------------------------- */

void Mixer::processLineIn(const model::Mixer& mixer, const mcl::AudioBuffer& inBuf,
    float inVol, float recTriggerLevel, bool isSeqActive) const
{
	/* Prepare the working buffer for input stream, which will be processed 
	later on by the Master Input Channel with plug-ins. */

	assert(inBuf.countChannels() <= mixer.getInBuffer().countChannels());

	mixer.getInBuffer().set(inBuf, inVol);

	const Peak peak = makePeak(mixer.getInBuffer());

	if (thresholdReached(peak, recTriggerLevel) && !m_signalCbFired && isSeqActive)
	{
		m_signalCbFired = true;
		onSignalTresholdReached();
		G_DEBUG("Signal > threshold!", );
	}

	mixer.a_setPeakIn(peak);
}

/* -------------------------------------------------------------------------- */

void Mixer::limit(mcl::AudioBuffer& outBuf) const
{
	for (int i = 0; i < outBuf.countFrames(); i++)
		for (int j = 0; j < outBuf.countChannels(); j++)
			outBuf[i][j] = std::max(-1.0f, std::min(outBuf[i][j], 1.0f));
}

/* -------------------------------------------------------------------------- */

void Mixer::finalizeOutput(const model::Mixer& mixer, mcl::AudioBuffer& buf,
    bool inToOut, bool shouldLimit, float vol) const
{
	if (inToOut)
		buf.sum(mixer.getInBuffer(), vol);
	else
		buf.applyGain(vol);

	if (shouldLimit)
		limit(buf);

	mixer.a_setPeakOut({buf.getPeak(CH_LEFT), buf.getPeak(CH_RIGHT)});
}
} // namespace giada::m
