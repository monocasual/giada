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

#ifndef G_MIDI_DISPATCHER_H
#define G_MIDI_DISPATCHER_H

#include "core/midiEvent.h"
#include "core/model/model.h"
#include "core/types.h"
#include <cstdint>
#include <functional>

namespace giada::m::midiDispatcher
{
void startChannelLearn(int param, ID channelId, std::function<void()> f);
void startMasterLearn(int param, std::function<void()> f);
void stopLearn();
void clearMasterLearn(int param, std::function<void()> f);
void clearChannelLearn(int param, ID channelId, std::function<void()> f);
#ifdef WITH_VST
void startPluginLearn(std::size_t paramIndex, ID pluginId, std::function<void()> f);
void clearPluginLearn(std::size_t paramIndex, ID pluginId, std::function<void()> f);
#endif

/* dispatch
Main callback invoked by kernelMidi whenever a new MIDI data comes in. */

void dispatch(int byte1, int byte2, int byte3);

/* learn
Learns event 'e'. Called by the Event Dispatcher. */

void learn(const MidiEvent& e);

/* process
Sends event 'e' to channels (masters and keyboard). Called by the Event 
Dispatcher. */

void process(const MidiEvent& e);

void setSignalCallback(std::function<void()> f);
} // namespace giada::m::midiDispatcher

#endif
