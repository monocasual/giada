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

#ifndef G_MODEL_TRACKS_H
#define G_MODEL_TRACKS_H

#include "src/core/model/track.h"

namespace giada::m
{
class Channel;
}

namespace giada::m::model
{
class Tracks
{
public:
	const std::vector<Track>&   getAll() const;
	const Channel&              getChannel(ID) const;
	bool                        anyChannelOf(std::function<bool(const Channel&)> f) const;
	std::vector<const Channel*> getChannels() const;

#if G_DEBUG_MODE
	void debug() const;
#endif

	/* add (1)
	Adds a new Track with an already available GroupChannel in it. */

	Track& add(Channel&& groupChannel, int width, bool internal);

	/* Add (2)
	Add an empty Track. Used while de-serializing. */

	Track& add(int width, bool internal);

	void                  remove(std::size_t index);
	Track&                get(std::size_t index);
	Channel&              getChannel(ID);
	Track&                getByChannel(ID);
	void                  addChannel(Channel&&, std::size_t index);
	void                  addChannel(Channel&&, std::size_t index, std::size_t position);
	void                  removeChannel(ID);
	Channel&              getLastChannel(std::size_t index);
	void                  forEachChannel(std::function<bool(Channel&)>);
	std::vector<Channel*> getChannelsIf(std::function<bool(const Channel&)>);

private:
	std::vector<Track> m_tracks;
};
} // namespace giada::m::model

#endif
