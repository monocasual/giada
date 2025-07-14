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

#ifndef G_RENDERING_MIDI_REACTIONS_H
#define G_RENDERING_MIDI_REACTIONS_H

#include "src/core/midiEvent.h"
#include "src/core/midiMapper.h"
#include "src/core/types.h"
#include "src/core/weakAtomic.h"
#include "src/types.h"

namespace giada::m
{
struct Action;
class KernelMidi;
class ActionRecorder;
class MidiLightning;
} // namespace giada::m

namespace giada::m::rendering
{
/* [play|stop|rewind]MidiChannel
Actions manually performed on a MIDI channel. */

void playMidiChannel(WeakAtomic<ChannelStatus>&);
void stopMidiChannel(WeakAtomic<ChannelStatus>&);
void rewindMidiChannel(WeakAtomic<ChannelStatus>&);

/* recordMidiAction
Records a new Action for a MIDI channel. */

void recordMidiAction(ID channelId, const MidiEvent&, Frame currentFrameQuantized, ActionRecorder&);
} // namespace giada::m::rendering

#endif
