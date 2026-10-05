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

#include "src/core/model/mixer.h"
#include "src/const.h"
#if G_DEBUG_MODE
#include <fmt/core.h>
#endif

namespace giada::m::model
{
namespace
{
/* updatePeak_
Atomically raises the shared peak only if the new value is higher. This lets the
audio thread accumulate the maximum peak between UI repaints (i.e. calls to
getPeak() below), so short peaks aren't lost before the UI reads and resets the
value. */

void updatePeak_(WeakAtomic<float>& peak, float value)
{
	float old = peak.load();
	while (old < value && !peak.compareExchange(old, value))
	{
	}
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

bool Mixer::Parameters::isActive() const
{
	return m_active.load() == true;
}

/* -------------------------------------------------------------------------- */

Frame Mixer::Parameters::getInputTracker() const
{
	return m_inputTracker.load();
}

/* -------------------------------------------------------------------------- */

void Mixer::Parameters::setActive(bool isActive) const
{
	m_active.store(isActive);
}

/* -------------------------------------------------------------------------- */

void Mixer::Parameters::setInputTracker(Frame f) const
{
	m_inputTracker.store(f);
}

/* -------------------------------------------------------------------------- */

Peak Mixer::Parameters::getPeakOut() const
{
	/* Atomically fetch the current L/R peaks and reset them to 0. exchange()
	does both in one step, so we consume the maximum accumulated since the last
	call and immediately start a new measurement window. This trick, paired with
	the setter-side logic in updatePeak_() above, turns each interval between
	calls to this method (done by the UI) into a small window that captures the
	highest peak seen in that time span, instead of just the most recently written
	block value. */
	return {
	    m_peakOutL.exchange(0.0f),
	    m_peakOutR.exchange(0.0f)};
}

Peak Mixer::Parameters::getPeakIn() const
{
	/* Same as above. */
	return {
	    m_peakInL.exchange(0.0f),
	    m_peakInR.exchange(0.0f)};
}

/* -------------------------------------------------------------------------- */

void Mixer::Parameters::setPeakOut(Peak p) const
{
	updatePeak_(m_peakOutL, p.left);
	updatePeak_(m_peakOutR, p.right);
}

void Mixer::Parameters::setPeakIn(Peak p) const
{
	updatePeak_(m_peakInL, p.left);
	updatePeak_(m_peakInR, p.right);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

#if G_DEBUG_MODE

void Mixer::debug() const
{
	puts("model::mixer");
	fmt::print("\thasSolos={}\n", hasSolos);
	fmt::print("\tisRecordingActions={}\n", isRecordingActions);
	fmt::print("\tisRecordingInput={}\n", isRecordingInput);
	fmt::print("\tinToOut={}\n", inToOut);
	fmt::print("\trenderPreview={}\n", renderPreview);
	fmt::print("\tinputRecMode={}\n", (int)inputRecMode);
	fmt::print("\trecTriggerMode={}\n", (int)recTriggerMode);
}

#endif
} // namespace giada::m::model
