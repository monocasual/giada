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

#ifndef G_MODEL_MIXER_H
#define G_MODEL_MIXER_H

#include "src/const.h"
#include "src/core/types.h"
#include "src/core/weakAtomic.h"
#include "src/deps/mcl-audio-buffer/src/audioBuffer.hpp"
#include "src/types.h"
#include <atomic>

namespace giada::m::model
{
class Mixer
{
public:
	class Parameters
	{
	public:
		bool  isActive() const;
		Frame getInputTracker() const;
		Peak  getPeakOut() const;
		Peak  getPeakIn() const;

		void setActive(bool) const;
		void setInputTracker(Frame) const;
		void setPeakOut(Peak) const;
		void setPeakIn(Peak) const;

	private:
		mutable WeakAtomic<bool>  m_active       = false;
		mutable WeakAtomic<float> m_peakOutL     = 0.0f;
		mutable WeakAtomic<float> m_peakOutR     = 0.0f;
		mutable WeakAtomic<float> m_peakInL      = 0.0f;
		mutable WeakAtomic<float> m_peakInR      = 0.0f;
		mutable WeakAtomic<Frame> m_inputTracker = 0;
	};

#if G_DEBUG_MODE
	void debug() const;
#endif

	bool           hasSolos           = false;
	bool           isRecordingActions = false;
	bool           isRecordingInput   = false;
	bool           inToOut            = false;
	bool           renderPreview      = false;
	InputRecMode   inputRecMode       = InputRecMode::RIGID;
	RecTriggerMode recTriggerMode     = RecTriggerMode::NORMAL;
};
} // namespace giada::m::model

#endif
