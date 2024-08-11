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

#include "core/model/track.h"
#include "core/types.h"
#include <cassert>
#ifdef G_DEBUG_MODE
#include <fmt/core.h>
#endif

namespace giada::m::model
{
Track::Track(std::size_t index, int width, bool internal)
: width(width)
, m_index(index)
, m_internal(internal)
{
}

/* -------------------------------------------------------------------------- */

const Channels& Track::getChannels() const
{
	return m_channels;
}

Channels& Track::getChannels()
{
	return m_channels;
}

/* -------------------------------------------------------------------------- */

std::size_t Track::getIndex() const
{
	return m_index;
}

/* -------------------------------------------------------------------------- */

std::size_t Track::getChannelIndex(ID channelId) const
{
	return m_channels.getIndex(channelId);
}

/* -------------------------------------------------------------------------- */

bool Track::isInternal() const
{
	return m_internal;
}

/* -------------------------------------------------------------------------- */

#ifdef G_DEBUG_MODE

void Track::debug() const
{
	fmt::print("model::track - index={} internal={}\n", m_index, m_internal);
	m_channels.debug();
}

#endif

/* -------------------------------------------------------------------------- */

const Channel* Track::findChannel(ID channelId) const
{
	return m_channels.find(channelId);
}

Channel* Track::findChannel(ID channelId)
{
	return m_channels.find(channelId);
}

/* -------------------------------------------------------------------------- */

const Channel& Track::getGroupChannel() const
{
	assert(m_channels.getAll().size() > 0);
	assert(m_channels.getAll()[0].type == ChannelType::GROUP);

	return m_channels.getAll()[0];
}

Channel& Track::getGroupChannel()
{
	return const_cast<Channel&>(std::as_const(*this).getGroupChannel());
}

/* -------------------------------------------------------------------------- */

std::size_t Track::getNumChannels() const
{
	return m_channels.getAll().size();
}

/* -------------------------------------------------------------------------- */

void Track::addChannel(Channel&& ch)
{
	m_channels.add(std::move(ch));
}

void Track::addChannel(Channel&& ch, std::size_t position)
{
	m_channels.add(std::move(ch), position);
}

/* -------------------------------------------------------------------------- */

Channel& Track::getLastChannel()
{
	return m_channels.getLast();
}
} // namespace giada::m::model
