/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "core/model/actions.h"
#include "core/model/behaviors.h"
#include "core/model/channels.h"
#include "core/model/kernelAudio.h"
#include "core/model/kernelMidi.h"
#include "core/model/midiIn.h"
#include "core/model/mixer.h"
#include "core/model/sequencer.h"

namespace giada::m::model
{
struct Document
{
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
	Channels    channels;
	Actions     actions;
	Behaviors   behaviors;
};
} // namespace giada::m::model

#endif
