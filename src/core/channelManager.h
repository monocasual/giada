/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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


#ifndef G_CHANNEL_MANAGER_H
#define G_CHANNEL_MANAGER_H


#include <string>


class Channel;
class SampleChannel;
class MidiChannel;


namespace giada {
namespace m {
namespace channelManager
{
int create(int type, int bufferSize, bool inputMonitorOn, Channel** out);

int  writePatch(const Channel* ch, bool isProject);
void writePatch(const SampleChannel* ch, bool isProject, int index);
void writePatch(const MidiChannel* ch, bool isProject, int index);

void readPatch(Channel* ch, int index);
void readPatch(SampleChannel* ch, const std::string& basePath, int index);
void readPatch(MidiChannel* ch, int index);
}}}; // giada::m::channelManager


#endif