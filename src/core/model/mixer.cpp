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

#include "core/model/mixer.h"

namespace giada::m::model
{
bool Mixer::a_isActive() const
{
	return state->active.load() == true;
}

/* -------------------------------------------------------------------------- */

Frame Mixer::a_getInputTracker() const
{
	return state->inputTracker.load();
}

/* -------------------------------------------------------------------------- */

void Mixer::a_setActive(bool isActive) const
{
	state->active.store(isActive);
}

/* -------------------------------------------------------------------------- */

void Mixer::a_setInputTracker(Frame f) const
{
	state->inputTracker.store(f);
}

/* -------------------------------------------------------------------------- */

Peak Mixer::a_getPeakOut() const
{
	return {state->peakOutL.load(), state->peakOutR.load()};
}

Peak Mixer::a_getPeakIn() const
{
	return {state->peakInL.load(), state->peakInR.load()};
}

/* -------------------------------------------------------------------------- */

void Mixer::a_setPeakOut(Peak p) const
{
	state->peakOutL.store(p.left);
	state->peakOutR.store(p.right);
}

void Mixer::a_setPeakIn(Peak p) const
{
	state->peakInL.store(p.left);
	state->peakInR.store(p.right);
}

/* -------------------------------------------------------------------------- */

mcl::AudioBuffer& Mixer::getRecBuffer() const { return buffer->rec; }
mcl::AudioBuffer& Mixer::getInBuffer() const { return buffer->in; }
} // namespace giada::m::model
