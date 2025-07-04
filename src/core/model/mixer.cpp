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

#include "src/core/model/mixer.h"
#ifdef G_DEBUG_MODE
#include <fmt/core.h>
#endif

namespace giada::m::model
{
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
	return {shared->peakOutL.load(), shared->peakOutR.load()};
}

Peak Mixer::a_getPeakIn() const
{
	return {shared->peakInL.load(), shared->peakInR.load()};
}

/* -------------------------------------------------------------------------- */

void Mixer::a_setPeakOut(Peak p) const
{
	shared->peakOutL.store(p.left);
	shared->peakOutR.store(p.right);
}

void Mixer::a_setPeakIn(Peak p) const
{
	shared->peakInL.store(p.left);
	shared->peakInR.store(p.right);
}

/* -------------------------------------------------------------------------- */

mcl::AudioBuffer& Mixer::getRecBuffer() const { return shared->recBuffer; }
mcl::AudioBuffer& Mixer::getInBuffer() const { return shared->inBuffer; }

/* -------------------------------------------------------------------------- */

#ifdef G_DEBUG_MODE

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
