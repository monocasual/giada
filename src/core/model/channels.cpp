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

#include "core/model/channels.h"
#include "utils/vector.h"
#include <cassert>
#ifdef G_DEBUG_MODE
#include <fmt/core.h>
#endif

namespace giada::m::model
{
Channel& Channels::get(ID id)
{
	return const_cast<Channel&>(std::as_const(*this).get(id));
}

const Channel& Channels::get(ID id) const
{
	auto it = std::find_if(m_channels.begin(), m_channels.end(), [id](const Channel& c) { return c.id == id; });
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

std::vector<const Channel*> Channels::getColumn(ID columnId) const
{
	std::vector<const Channel*> out;
	for (const Channel& ch : m_channels)
		if (ch.columnId == columnId)
			out.push_back(&ch);
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
		const Wave* wave = c.sampleChannel ? c.sampleChannel->getWave() : nullptr;

		fmt::print("\t{} - ID={} name='{}' type={} columnId={} position={} channelShared={} wave={}\n",
		    i++, c.id, c.name, (int)c.type, c.columnId, c.position, (void*)&c.shared, (void*)wave);

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
	u::vector::removeIf(m_channels, [id](const Channel& c) { return c.id == id; });
}

/* -------------------------------------------------------------------------- */

void Channels::add(const Channel& ch)
{
	m_channels.push_back(ch);
}
} // namespace giada::m::model
