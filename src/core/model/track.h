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

#ifndef G_MODEL_TRACK_H
#define G_MODEL_TRACK_H

#include "src/core/model/channels.h"

namespace giada::m::model
{
class Track
{
	friend class Tracks;

public:
	/* ctor
	A non-internal track always contains at least one Group Channel that determines the
	track's behavior and properties (volumes, FXs, ...). */

	Track(std::size_t index, int width, bool internal);

	const Channels& getChannels() const;
	const Channel*  findChannel(ID) const;
	const Channel&  getGroupChannel() const;
	std::size_t     getNumChannels() const;

	/* getIndex
	Returns this Track index. */

	std::size_t getIndex() const;

	/* getChannelIndex
	Returns the Channel index given its ID. */

	std::size_t getChannelIndex(ID) const;

	/* isInternal
	True when the Track should be hidden, containing only master i/o or preview
	channels, not to be shown on the UI. */

	bool isInternal() const;

#if G_DEBUG_MODE
	void debug() const;
#endif

	Channel*  findChannel(ID);
	Channel&  getGroupChannel();
	void      addChannel(Channel&&);
	void      addChannel(Channel&&, std::size_t position);
	Channel&  getLastChannel();
	Channels& getChannels();
	void      removeChannel(ID);

	int width;

private:
	Channels    m_channels;
	std::size_t m_index;
	bool        m_internal;
};
} // namespace giada::m::model

#endif
