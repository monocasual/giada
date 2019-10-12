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


#include "masterChannel.h"


namespace giada {
namespace m 
{
MasterChannel::MasterChannel(int bufferSize, ID id)
: Channel(ChannelType::MASTER, ChannelStatus::OFF, bufferSize, 0, id)
{
}


/* -------------------------------------------------------------------------- */


MasterChannel::MasterChannel(const patch::Channel& p, int bufferSize)
: Channel(p, bufferSize)
{
}


/* -------------------------------------------------------------------------- */


MasterChannel* MasterChannel::clone() const
{
	return new MasterChannel(*this);
}

	
/* -------------------------------------------------------------------------- */


void MasterChannel::load(const patch::Channel& p)
{
	volume    = p.volume;
#ifdef WITH_VST
    pluginIds = p.pluginIds;
#endif
}


/* -------------------------------------------------------------------------- */


void MasterChannel::render(AudioBuffer& out, const AudioBuffer& in, 
	AudioBuffer& inToOut, bool audible, bool running)
{
#ifdef WITH_VST
	if (pluginIds.size() == 0)
		return;
	if (id == mixer::MASTER_OUT_CHANNEL_ID)
		pluginHost::processStack(out, pluginIds);
	else
	if (id == mixer::MASTER_IN_CHANNEL_ID)
		pluginHost::processStack(inToOut, pluginIds);
#endif
}

}} // giada::m::
