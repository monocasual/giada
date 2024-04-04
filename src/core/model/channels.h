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

#ifndef G_MODEL_CHANNELS_H
#define G_MODEL_CHANNELS_H

#include "core/channels/channel.h"
#include "core/types.h"
#include "utils/container.h"

namespace giada::m::model
{
class Channels : public u::Container<Channel, /*Identifiable=*/true, /*Sortable=*/true>
{
public:
	/* get
	Returns a Channel for the given ID, by looking also inside groups. */

	const Channel& get(ID) const;
	Channel&       get(ID);

#ifdef G_DEBUG_MODE
	void debug() const;
#endif

private:
	const Channel* find_(ID) const;
};
} // namespace giada::m::model

#endif
