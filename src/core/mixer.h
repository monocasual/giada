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

#ifndef G_MIXER_H
#define G_MIXER_H

#include "src/core/midiEvent.h"
#include "src/core/ringBuffer.h"
#include "src/core/sequencer.h"
#include "src/core/types.h"
#include "src/core/weakAtomic.h"
#include "src/deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include <functional>

namespace mcl
{
class AudioBuffer;
}

namespace giada::m::model
{
class Mixer;
class Channels;
struct Document;
} // namespace giada::m::model

namespace giada::m
{
struct Action;
class Channel;
class Mixer
{
public:
	/* RecordInfo
	Information regarding the input recording progress. */

	struct RecordInfo
	{
		Frame position;
		int   maxLength;
	};

	Mixer(model::Model&);

	Peak getPeakOut() const;
	Peak getPeakIn() const;
	bool getInToOut() const;

	/* getRecordInfo
	Returns information on the ongoing input recording. */

	RecordInfo getRecordInfo() const;

	bool isRecordingActions() const;
	bool isRecordingInput() const;

	RecTriggerMode getRecTriggerMode() const;
	InputRecMode   getInputRecMode() const;

	/* render
	Core rendering function. */

	void render(const mcl::AudioBuffer& in, const model::Document&, int maxFramesToRec) const;

	/* reset
	Brings everything back to the initial state. Must be called only when mixer
	is disabled.*/

	void reset(int framesInLoop, int framesInBuffer);

	/* enable, disable
	Toggles master callback processing. Useful to suspend the rendering. */

	void enable();
	void disable();

	/* allocRecBuffer
	Allocates new memory for the virtual input channel. */

	void allocRecBuffer(int frames);

	/* clearRecBuffer
	Clears internal virtual channel. */

	void clearRecBuffer();

	/* getRecBuffer
	Returns a read-only reference to the internal virtual channel. Use this to
	merge data into channel after an input recording session. */

	const mcl::AudioBuffer& getRecBuffer();

	/* startInputRec, stopInputRec
	Starts/stops input recording on frame 'from'. The latter returns the frame
	where the recording ended. */

	void  startInputRec(Frame from);
	Frame stopInputRec();

	void startActionRec();
	void stopActionRec();

	/* updateSoloCount
	Updates the number of solo-ed channels in mixer. */

	void updateSoloCount(bool hasSolos);

	/* setInToOut
	Toggles the InToOut flag, a.k.a. the 'hear-what-you're-playing' feature. */

	void setInToOut(bool v);

	/* finalizeOutput
	Last touches after the output has been rendered: apply inToOut if any, apply
	output volume. */

	void finalizeOutput(const model::Mixer&, mcl::AudioBuffer&, bool inToOut,
	    bool limit, float vol) const;

	/* updateOutputPeak
	Reads the maximum peak in the given buffer and updates the value in model::Mixer. */

	void updateOutputPeak(const model::Mixer&, const mcl::AudioBuffer&) const;

	void setRecTriggerMode(RecTriggerMode);
	void setInputRecMode(InputRecMode);

	/* onSignalTresholdReached
	Callback fired when audio has reached a certain threshold (record-on-signal
	mode). */

	std::function<void()> onSignalTresholdReached;

	/* onEndOfRecording
	Callback fired when the audio recording session has ended. */

	std::function<void()> onEndOfRecording;

private:
	/* thresholdReached
	Returns true if left or right channel's peak has reached a certain
	threshold. */

	bool thresholdReached(Peak p, float threshold) const;

	/* makePeak
	Returns a Peak object given an audio buffer, taking number of channels into
	account. */

	Peak makePeak(const mcl::AudioBuffer& b) const;

	/* lineInRec
	Records from line in. 'maxFrames' determines how many frames to record
	before the internal tracker loops over. The value changes whether you are
	recording in RIGID or FREE mode. Returns the number of recorded frames. */

	int lineInRec(const mcl::AudioBuffer& inBuf, mcl::AudioBuffer& recBuf,
	    Frame inputTracker, int maxFrames, float inVol, bool allowsOverdub) const;

	/* processLineIn
	Computes line in peaks and prepares the internal working buffer for input
	recording. */

	void processLineIn(const model::Mixer& mixer, const mcl::AudioBuffer& inBuf,
	    float inVol, float recTriggerLevel, bool isSeqActive) const;

	/* limit
	Applies a very dumb hard limiter. */

	void limit(mcl::AudioBuffer& outBuf) const;

	model::Model& m_model;

	/* m_signalCbFired, m_endOfRecCbFired
	Boolean guards to determine whether the callbacks have been fired or not,
	to avoid retriggering. Mutable: strictly for internal use only. */

	mutable bool m_signalCbFired;
	mutable bool m_endOfRecCbFired;
};
} // namespace giada::m

#endif
