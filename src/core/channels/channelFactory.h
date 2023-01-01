/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_CHANNEL_FACTORY_H
#define G_CHANNEL_FACTORY_H

#include "core/channels/channel.h"
#include "core/idManager.h"
#include "core/patch.h"
#include "core/types.h"

namespace giada::m
{
class Wave;
class ChannelFactory final
{
public:
	struct Data
	{
		Channel                        channel;
		std::unique_ptr<ChannelShared> shared;
	};

	/* getNextId
	Returns the next channel ID that will be assigned to a new channel. */

	static ID getNextId();

	/* reset
    Resets internal ID generator. */

	static void reset();

	/* create (1)
    Creates a new channel. If channelId == 0 generates a new ID, reuse the one 
    passed in otherwise. */

	static Data create(ID channelId, ChannelType type, ID columnId, int position, int bufferSize, Resampler::Quality, bool overdubProtection);

	/* create (2)
    Creates a new channel given an existing one (i.e. clone). */

	static Data create(const Channel& ch, int bufferSize, Resampler::Quality);

	/* (de)serializeWave
    Creates a new channel given the patch raw data and vice versa. */

	static Data                 deserializeChannel(const Patch::Channel& c, float samplerateRatio, int bufferSize, Resampler::Quality, Wave*);
	static const Patch::Channel serializeChannel(const Channel& c);

private:
	static std::unique_ptr<ChannelShared> makeShared(ChannelType type, int bufferSize, Resampler::Quality);

	static IdManager m_channelId;
};
} // namespace giada::m

#endif
