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

#include "src/core/model/channels.h"
#include "src/core/plugins/plugin.h"
#include "src/utils/vector.h"
#include <cassert>
#ifdef G_DEBUG_MODE
#include "src/utils/string.h"
#include <fmt/core.h>
#endif

namespace giada::m::model
{
Channel* Channels::find(ID id)
{
	return const_cast<Channel*>(std::as_const(*this).find(id));
}

const Channel* Channels::find(ID id) const
{
	auto it = std::find_if(m_channels.begin(), m_channels.end(), [id](const Channel& c)
	{ return c.id == id; });
	return it != m_channels.end() ? &*it : nullptr;
}

/* -------------------------------------------------------------------------- */

Channel& Channels::get(ID id)
{
	return const_cast<Channel&>(std::as_const(*this).get(id));
}

const Channel& Channels::get(ID id) const
{
	auto it = std::find_if(m_channels.begin(), m_channels.end(), [id](const Channel& c)
	{ return c.id == id; });
	assert(it != m_channels.end());
	return *it;
}

/* -------------------------------------------------------------------------- */

Channel& Channels::getLast()
{
	return m_channels.back();
}

/* -------------------------------------------------------------------------- */

std::vector<Channel>& Channels::getAll()
{
	return m_channels;
}

const std::vector<Channel>& Channels::getAll() const
{
	return m_channels;
}

/* -------------------------------------------------------------------------- */

const std::size_t Channels::getIndex(ID id) const
{
	return static_cast<std::size_t>(u::vector::indexOf(m_channels, get(id)));
}

/* -------------------------------------------------------------------------- */

const std::vector<ID> Channels::getAllIDs() const
{
	std::vector<ID> out;
	for (const Channel& ch : m_channels)
		out.push_back(ch.id);
	return out;
}

/* -------------------------------------------------------------------------- */

bool Channels::anyOf(std::function<bool(const Channel&)> f) const
{
	return std::any_of(m_channels.begin(), m_channels.end(), f);
}

/* -------------------------------------------------------------------------- */

#ifdef G_DEBUG_MODE

void Channels::debug() const
{
	puts("model::channels");

	for (int i = 0; const Channel& c : m_channels)
	{
		fmt::print("\t{} - {}\n", i++, c.debug());

		if (c.plugins.size() > 0)
		{
			puts("\t\tplugins:");
			for (const auto& p : c.plugins)
				fmt::print("\t\t\t{} - ID={}\n", (void*)p, p->id);
		}
	}
}

#endif

/* -------------------------------------------------------------------------- */

std::vector<Channel*> Channels::getIf(std::function<bool(const Channel&)> f)
{
	std::vector<Channel*> out;
	for (Channel& ch : m_channels)
		if (f(ch))
			out.push_back(&ch);
	return out;
}

/* -------------------------------------------------------------------------- */

void Channels::remove(ID id)
{
	u::vector::removeIf(m_channels, [id](const Channel& c)
	{ return c.id == id; });
}

/* -------------------------------------------------------------------------- */

void Channels::add(Channel&& ch)
{
	m_channels.push_back(std::move(ch));
}

/* -------------------------------------------------------------------------- */

void Channels::add(Channel&& ch, std::size_t position)
{
	m_channels.insert(m_channels.begin() + std::min(position, m_channels.size()), std::move(ch));
}
} // namespace giada::m::model
