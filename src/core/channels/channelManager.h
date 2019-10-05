/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#include <memory>
#include "core/types.h"


namespace giada {
namespace m 
{
namespace patch
{
struct Channel;
}
class Channel;
class SampleChannel;
class MidiChannel;
namespace channelManager
{
/* create (1)
Creates a new Channel from scratch. */

std::unique_ptr<Channel> create(ChannelType type, int bufferSize,
	bool inputMonitorOn, ID columnId);

/* create (2)
Creates a new Channel given an existing one (i.e. clone). */

std::unique_ptr<Channel> create(const Channel& ch);

/* create (3)
Creates a new Channel out of a patch::Channel. */

std::unique_ptr<Channel> create(const patch::Channel& c, int bufferSize);
}}}; // giada::m::channelManager


#endif
