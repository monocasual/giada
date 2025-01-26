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

#ifndef G_MODEL_MIXER_H
#define G_MODEL_MIXER_H

#include "core/const.h"
#include "core/types.h"
#include "core/weakAtomic.h"
#include "deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include <atomic>

namespace giada::m::model
{
class Mixer
{
	friend class Model;
	friend class Shared;

public:
	bool  a_isActive() const;
	Frame a_getInputTracker() const;
	Peak  a_getPeakOut() const;
	Peak  a_getPeakIn() const;

	void a_setActive(bool) const;
	void a_setInputTracker(Frame) const;
	void a_setPeakOut(Peak) const;
	void a_setPeakIn(Peak) const;

	mcl::AudioBuffer& getRecBuffer() const;
	mcl::AudioBuffer& getInBuffer() const;

#ifdef G_DEBUG_MODE
	void debug() const;
#endif

	bool           hasSolos           = false;
	bool           isRecordingActions = false;
	bool           isRecordingInput   = false;
	bool           inToOut            = false;
	bool           renderPreview      = false;
	InputRecMode   inputRecMode       = InputRecMode::RIGID;
	RecTriggerMode recTriggerMode     = RecTriggerMode::NORMAL;

private:
	struct Shared
	{
		Shared& operator=(const Shared&);

		std::atomic<bool> active       = false;
		WeakAtomic<float> peakOutL     = 0.0f;
		WeakAtomic<float> peakOutR     = 0.0f;
		WeakAtomic<float> peakInL      = 0.0f;
		WeakAtomic<float> peakInR      = 0.0f;
		WeakAtomic<Frame> inputTracker = 0;

		/* recBuffer
		Working buffer for audio recording. */

		mcl::AudioBuffer recBuffer;

		/* inBuffer
		Working buffer for input channel. Used for the in->out bridge. */

		mcl::AudioBuffer inBuffer;
	};

	Shared* shared = nullptr;
};
} // namespace giada::m::model

#endif
