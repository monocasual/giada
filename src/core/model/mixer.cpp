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

void updatePeak_(std::atomic<float>& peak, float value)
{
	float old = peak.load(std::memory_order_relaxed);
	while (old < value && !peak.compare_exchange_weak(old, value, std::memory_order_relaxed))
	{
	}
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

Mixer::Shared& Mixer::Shared::operator=(const Mixer::Shared& o)
{
	if (this == &o)
		return *this;
	active.store(o.active.load());
	peakOutL.store(0.0f);
	peakOutR.store(0.0f);
	peakInL.store(0.0f);
	peakInR.store(0.0f);
	inputTracker.store(0);
	return *this;
}

/* -------------------------------------------------------------------------- */

bool Mixer::a_isActive() const
{
	return shared->active.load() == true;
}

/* -------------------------------------------------------------------------- */

Frame Mixer::a_getInputTracker() const
{
	return shared->inputTracker.load();
}

/* -------------------------------------------------------------------------- */

void Mixer::a_setActive(bool isActive) const
{
	shared->active.store(isActive);
}

/* -------------------------------------------------------------------------- */

void Mixer::a_setInputTracker(Frame f) const
{
	shared->inputTracker.store(f);
}

/* -------------------------------------------------------------------------- */

Peak Mixer::a_getPeakOut() const
{
	/* Atomically fetch the current L/R peaks and reset them to 0. exchange()
	does both in one step, so we consume the maximum accumulated since the last
	call and immediately start a new measurement window. This trick, paired with
	the setter-side logic in updatePeak_() above, turns each interval between
	calls to this method (done by the UI) into a small window that captures the
	highest peak seen in that time span, instead of just the most recently written
	block value. */
	return {
	    shared->peakOutL.exchange(0.0f, std::memory_order_relaxed),
	    shared->peakOutR.exchange(0.0f, std::memory_order_relaxed)};
}

Peak Mixer::a_getPeakIn() const
{
	/* Same as above. */
	return {
	    shared->peakInL.exchange(0.0f, std::memory_order_relaxed),
	    shared->peakInR.exchange(0.0f, std::memory_order_relaxed)};
}

/* -------------------------------------------------------------------------- */

void Mixer::a_setPeakOut(Peak p) const
{
	updatePeak_(shared->peakOutL, p.left);
	updatePeak_(shared->peakOutR, p.right);
}

void Mixer::a_setPeakIn(Peak p) const
{
	updatePeak_(shared->peakInL, p.left);
	updatePeak_(shared->peakInR, p.right);
}

/* -------------------------------------------------------------------------- */

mcl::AudioBuffer& Mixer::getRecBuffer() const { return shared->recBuffer; }
mcl::AudioBuffer& Mixer::getInBuffer() const { return shared->inBuffer; }

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
