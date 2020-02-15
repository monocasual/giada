/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#include <functional>
#include <cstdint>
#include "core/model/model.h"
#include "core/midiEvent.h"
#include "core/types.h"


namespace giada {
namespace m {
namespace midiDispatcher
{
void startChannelLearn(int param, ID channelId, std::function<void()> f);
void startMasterLearn (int param, std::function<void()> f);
void stopLearn();
void clearMasterLearn (int param, std::function<void()> f);
void clearChannelLearn(int param, ID channelId, std::function<void()> f);
#ifdef WITH_VST
void startPluginLearn (int paramIndex, ID pluginId, std::function<void()> f);
void clearPluginLearn (int paramIndex, ID pluginId, std::function<void()> f);
#endif

void dispatch(int byte1, int byte2, int byte3);

void setSignalCallback(std::function<void()> f);
}}}; // giada::m::midiDispatcher::


#endif
