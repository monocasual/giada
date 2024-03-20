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

#include "core/model/channels.h"
#include "core/plugins/plugin.h"
#include "utils/vector.h"
#include <cassert>
#ifdef G_DEBUG_MODE
#include "utils/string.h"
#include <fmt/core.h>
#endif

namespace giada::m::model
{
const Channel& Channels::get(ID id) const
{
	const Channel* out = find_(id);
	assert(out != nullptr);
	return *out;
}

/* -------------------------------------------------------------------------- */

Channel& Channels::get(ID id)
{
	return const_cast<Channel&>(std::as_const(*this).get(id));
}

/* -------------------------------------------------------------------------- */

#ifdef G_DEBUG_MODE

void Channels::debug() const
{
	puts("model::channels");

	for (int i = 0; const Channel& c : getAll())
	{
		fmt::print("\t{} - {}\n", i++, c.debug());
		if (c.type == ChannelType::GROUP)
			for (const Channel& child : c.groupChannel->channels->getAll())
				fmt::print("\t\t{} - {}\n", i++, child.debug());

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

const Channel* Channels::find_(ID id) const
{
	for (const Channel& ch : getAll())
	{
		if (ch.id == id)
			return &ch;
		if (ch.type == ChannelType::GROUP)
		{
			const Channel* child = ch.groupChannel->channels->findById(id);
			if (child != nullptr)
				return child;
		}
	}
	return nullptr;
}
} // namespace giada::m::model
