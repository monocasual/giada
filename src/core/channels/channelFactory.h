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
#include "core/conf.h"
#include "core/idManager.h"
#include "core/patch.h"
#include "core/types.h"

namespace giada::m::model
{
class Model;
}

namespace giada::m
{
class KernelAudio;
class ChannelFactory final
{
public:
	ChannelFactory(const Conf::Data&, model::Model&);

	/* getNextId
	Returns the next channel ID that will be assigned to a new channel. */

	ID getNextId() const;

	/* reset
    Resets internal ID generator. */

	void reset();

	/* create (1)
    Creates a new channel. If channelId == 0 generates a new ID, reuse the one 
    passed in otherwise. */

	Channel create(ID channelId, ChannelType type, ID columnId, int bufferSize);

	/* create (2)
    Creates a new channel given an existing one (i.e. clone). */

	Channel create(const Channel& ch, int bufferSize);

	/* (de)serializeWave
    Creates a new channel given the patch raw data and vice versa. */

	Channel              deserializeChannel(const Patch::Channel& c, float samplerateRatio, int bufferSize);
	const Patch::Channel serializeChannel(const Channel& c);

private:
	ChannelShared& makeShared(ChannelType type, int bufferSize);

	IdManager m_channelId;

	const Conf::Data& m_conf;
	model::Model&     m_model;
};
} // namespace giada::m

#endif
