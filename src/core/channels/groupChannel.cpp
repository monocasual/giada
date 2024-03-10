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

#include "core/channels/groupChannel.h"
#include "core/model/channels.h"
#include "utils/vector.h"

namespace giada::m
{
const std::vector<ID>& GroupChannel::getChildren() const
{
	return m_children;
}

/* -------------------------------------------------------------------------- */

const bool GroupChannel::hasChild(ID targetId) const
{
	return u::vector::has(m_children, [targetId](ID channelId)
	    { return channelId == targetId; });
}

/* -------------------------------------------------------------------------- */

void GroupChannel::addChild(ID channelId)
{
	assert(!hasChild(channelId));

	m_children.push_back(channelId);
}

/* -------------------------------------------------------------------------- */

void GroupChannel::removeChild(ID channelId)
{
	assert(hasChild(channelId));

	u::vector::remove(m_children, channelId);
}
} // namespace giada::m
