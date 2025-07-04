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

#ifndef G_MODEL_DOCUMENT_H
#define G_MODEL_DOCUMENT_H

#include "src/core/model/actions.h"
#include "src/core/model/behaviors.h"
#include "src/core/model/channels.h"
#include "src/core/model/kernelAudio.h"
#include "src/core/model/kernelMidi.h"
#include "src/core/model/midiIn.h"
#include "src/core/model/mixer.h"
#include "src/core/model/sequencer.h"
#include "src/core/model/tracks.h"

namespace giada::m
{
struct Conf;
}

namespace giada::m::model
{
class Shared;
struct Document
{
	/* load (1)
	Loads data from a Patch object. */

	void load(const Patch&, Shared&, float sampleRateRatio);

	/* load (2)
	Loads data from a Conf object. */

	void load(const Conf&);

	/* store (1)
	Stores data into a Patch object. */

	void store(Patch&) const;

	/* store (2)
	Stores data into a Conf object. */

	void store(Conf&) const;

#ifdef G_DEBUG_MODE
	void debug() const;
#endif

	/* locked
	If locked, Mixer won't process channels. This is used to allow editing the
	shared data (e.g. Plugins, Waves) by the rendering engine without data races. */

	bool locked = false;

	KernelAudio kernelAudio;
	KernelMidi  kernelMidi;
	Sequencer   sequencer;
	Mixer       mixer;
	MidiIn      midiIn;
	Tracks      tracks;
	Actions     actions;
	Behaviors   behaviors;
};
} // namespace giada::m::model

#endif
