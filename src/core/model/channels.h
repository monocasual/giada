/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "core/channels/channel.h"
#include "core/types.h"

namespace giada::m::model
{
class Channels
{
	friend class Model;

public:
	const Channel&              get(ID) const;
	const std::vector<Channel>& getAll() const;

	/* getColumn
	Returns all channels that belongs to column 'columnId'. Read-only. */

	const std::vector<const Channel*> getColumn(ID columnId) const;

	/* anyOf
    Returns true if any channel satisfies the callback 'f'. */

	bool anyOf(std::function<bool(const Channel&)> f) const;

#ifdef G_DEBUG_MODE
	void debug() const;
#endif

	Channel&              get(ID);
	Channel&              getLast();
	std::vector<Channel>& getAll();
	std::vector<Channel*> getIf(std::function<bool(const Channel&)> f);
	void                  add(const Channel&);
	void                  remove(ID);
	void                  clear();

private:
	std::vector<Channel> m_channels;
};
} // namespace giada::m::model

#endif
