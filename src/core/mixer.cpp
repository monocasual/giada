/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

void Mixer::reset(Frame maxFramesInLoop, Frame framesInBuffer)
{
	/* Allocate working buffers. rec buffer has variable size: it depends on how
	many frames there are in the current loop. */

	m_model.get().mixer.getRecBuffer().alloc(maxFramesInLoop, G_MAX_IO_CHANS);
	m_model.get().mixer.getInBuffer().alloc(framesInBuffer, G_MAX_IO_CHANS);

	u::log::print("[mixer::reset] buffers ready - maxFramesInLoop=%d, framesInBuffer=%d\n",
	    maxFramesInLoop, framesInBuffer);
}

/* -------------------------------------------------------------------------- */

bool Mixer::isActive() const { return m_model.get().mixer.a_isActive(); }

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

void Mixer::allocRecBuffer(Frame frames)
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

void Mixer::advanceChannels(const Sequencer::EventBuffer& events, const model::Layout& rtLayout)
{
	for (const channel::Data& c : rtLayout.channels)
		if (!c.isInternal())
			channel::advance(c, events);
}

/* -------------------------------------------------------------------------- */

void Mixer::render(mcl::AudioBuffer& out, const mcl::AudioBuffer& in, const model::Layout& layout_RT) const
{
	const model::Mixer&     mixer     = layout_RT.mixer;
	const model::Sequencer& sequencer = layout_RT.sequencer;
	const model::Recorder&  recorder  = layout_RT.recorder;

	const channel::Data& masterOutCh = layout_RT.getChannel(Mixer::MASTER_OUT_CHANNEL_ID);
	const channel::Data& masterInCh  = layout_RT.getChannel(Mixer::MASTER_IN_CHANNEL_ID);
	const channel::Data& previewCh   = layout_RT.getChannel(Mixer::PREVIEW_CHANNEL_ID);

	const bool  hasInput        = in.isAllocd();
	const bool  inToOut         = mixer.inToOut;
	const bool  isSeqActive     = sequencer.isActive();
	const bool  shouldLineInRec = isSeqActive && recorder.a_isRecordingInput() && hasInput;
	const float recTriggerLevel = mixer.recTriggerLevel;
	const Frame maxFramesToRec  = mixer.maxFramesToRec;
	const bool  allowsOverdub   = mixer.allowsOverdub;
	const bool  limitOutput     = mixer.limitOutput;

	mixer.getInBuffer().clear();

	/* Reset peak computation. */

	mixer.a_setPeakOut({0.0f, 0.0f});
	mixer.a_setPeakIn({0.0f, 0.0f});

	if (hasInput)
	{
		processLineIn(mixer, in, masterInCh.volume, recTriggerLevel, isSeqActive);
		renderMasterIn(masterInCh, mixer.getInBuffer());
	}

	if (shouldLineInRec)
	{
		const Frame newTrackerPos = lineInRec(in, mixer.getRecBuffer(),
		    mixer.a_getInputTracker(), maxFramesToRec, masterInCh.volume,
		    allowsOverdub);
		mixer.a_setInputTracker(newTrackerPos);
	}

	/* Channel processing. Don't do it if layout is locked: another thread is 
	changing data (e.g. Plugins or Waves). */

	if (!layout_RT.locked)
		renderChannels(layout_RT.channels, out, mixer.getInBuffer());

	/* Render remaining internal channels. */

	renderMasterOut(masterOutCh, out);
	renderPreview(previewCh, out);

	/* Post processing. */

	finalizeOutput(mixer, out, inToOut, limitOutput, masterOutCh.volume);
}

/* -------------------------------------------------------------------------- */

void Mixer::startInputRec(Frame from)
{
	m_model.get().mixer.a_setInputTracker(from);
}

Frame Mixer::stopInputRec()
{
	const Frame ret = m_model.get().mixer.a_getInputTracker();
	m_model.get().mixer.a_setInputTracker(0);
	m_signalCbFired   = false;
	m_endOfRecCbFired = false;
	return ret;
}

/* -------------------------------------------------------------------------- */

bool Mixer::isChannelAudible(const channel::Data& c) const
{
	if (c.isInternal())
		return true;
	if (c.mute)
		return false;
	const bool hasSolos = m_model.get().mixer.hasSolos;
	return !hasSolos || (hasSolos && c.solo);
}

/* -------------------------------------------------------------------------- */

Peak Mixer::getPeakOut() const { return m_model.get().mixer.a_getPeakOut(); }
Peak Mixer::getPeakIn() const { return m_model.get().mixer.a_getPeakIn(); }

/* -------------------------------------------------------------------------- */

Mixer::RecordInfo Mixer::getRecordInfo() const
{
	return {
	    m_model.get().mixer.a_getInputTracker(),
	    m_model.get().mixer.getRecBuffer().countFrames()};
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

Frame Mixer::lineInRec(const mcl::AudioBuffer& inBuf, mcl::AudioBuffer& recBuf, Frame inputTracker,
    Frame maxFrames, float inVol, bool allowsOverdub) const
{
	assert(maxFrames > 0 && maxFrames <= recBuf.countFrames());
	assert(onEndOfRecording != nullptr);

	if (inputTracker >= maxFrames && !allowsOverdub && !m_endOfRecCbFired)
	{
		onEndOfRecording();
		m_endOfRecCbFired = true;
		return 0;
	}

	const Frame framesToCopy = -1; // copy everything
	const Frame srcOffset    = 0;
	const Frame destOffset   = inputTracker % maxFrames; // loop over at maxFrames

	recBuf.sum(inBuf, framesToCopy, srcOffset, destOffset, inVol);

	return inputTracker + inBuf.countFrames();
}

/* -------------------------------------------------------------------------- */

void Mixer::processLineIn(const model::Mixer& mixer, const mcl::AudioBuffer& inBuf,
    float inVol, float recTriggerLevel, bool isSeqActive) const
{
	const Peak peak = makePeak(inBuf);

	if (thresholdReached(peak, recTriggerLevel) && !m_signalCbFired && isSeqActive)
	{
		m_signalCbFired = true;
		onSignalTresholdReached();
		G_DEBUG("Signal > threshold!");
	}

	mixer.a_setPeakIn(peak);

	/* Prepare the working buffer for input stream, which will be processed 
	later on by the Master Input Channel with plug-ins. */

	assert(inBuf.countChannels() <= mixer.getInBuffer().countChannels());

	mixer.getInBuffer().set(inBuf, inVol);
}

/* -------------------------------------------------------------------------- */

void Mixer::renderChannels(const std::vector<channel::Data>& channels, mcl::AudioBuffer& out, mcl::AudioBuffer& in) const
{
	for (const channel::Data& c : channels)
		if (!c.isInternal())
			channel::render(c, &out, &in, isChannelAudible(c));
}

/* -------------------------------------------------------------------------- */

void Mixer::renderMasterIn(const channel::Data& ch, mcl::AudioBuffer& in) const
{
	channel::render(ch, nullptr, &in, true);
}

void Mixer::renderMasterOut(const channel::Data& ch, mcl::AudioBuffer& out) const
{
	channel::render(ch, &out, nullptr, true);
}

void Mixer::renderPreview(const channel::Data& ch, mcl::AudioBuffer& out) const
{
	channel::render(ch, &out, nullptr, true);
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
