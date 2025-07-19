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

#ifndef G_MODEL_CHANNELS_H
#define G_MODEL_CHANNELS_H

#include "src/core/channels/channel.h"
#include "src/core/types.h"

namespace giada::m::model
{
class Channels
{
public:
	const Channel&              get(ID) const;
	const Channel*              find(ID) const;
	const std::vector<Channel>& getAll() const;
	const std::size_t           getIndex(ID) const;
	const std::vector<ID>       getAllIDs() const;

	/* anyOf
	Returns true if any channel satisfies the callback 'f'. */

	bool anyOf(std::function<bool(const Channel&)> f) const;

#if G_DEBUG_MODE
	void debug() const;
#endif

	Channel*              find(ID);
	Channel&              get(ID);
	Channel&              getLast();
	std::vector<Channel>& getAll();
	std::vector<Channel*> getIf(std::function<bool(const Channel&)> f);
	void                  add(Channel&&);
	void                  add(Channel&&, std::size_t position);
	void                  remove(ID);

private:
	std::vector<Channel> m_channels;
};
} // namespace giada::m::model

#endif
