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

#include "src/core/model/tracks.h"
#include "src/utils/vector.h"

namespace giada::m::model
{
const std::vector<Track>& Tracks::getAll() const
{
	return m_tracks;
}

/* -------------------------------------------------------------------------- */

Track& Tracks::add(Channel&& groupChannel, int width, bool internal)
{
	Track& track = add(width, internal);
	track.addChannel(std::move(groupChannel));
	return track;
}

Track& Tracks::add(int width, bool internal)
{
	m_tracks.push_back({m_tracks.size(), width, internal});
	return m_tracks.back();
}

/* -------------------------------------------------------------------------- */

void Tracks::remove(std::size_t index)
{
	assert(index < m_tracks.size());

	m_tracks.erase(m_tracks.begin() + index);

	for (std::size_t index = 0; Track& track : m_tracks)
		track.m_index = index++;
}

/* -------------------------------------------------------------------------- */

Track& Tracks::get(std::size_t index)
{
	assert(index < m_tracks.size());

	return m_tracks[index];
}

/* -------------------------------------------------------------------------- */

const Channel& Tracks::getChannel(ID channelId) const
{
	const Channel* out = nullptr;
	for (const Track& track : m_tracks)
	{
		out = track.findChannel(channelId);
		if (out != nullptr)
			break;
	}
	assert(out != nullptr);
	return *out;
}

Channel& Tracks::getChannel(ID channelId)
{
	return const_cast<Channel&>(std::as_const(*this).getChannel(channelId));
}

/* -------------------------------------------------------------------------- */

void Tracks::forEachChannel(std::function<bool(Channel&)> f)
{
	for (Track& track : m_tracks)
		for (Channel& channel : track.getChannels().getAll())
			if (!f(channel))
				return;
}

/* -------------------------------------------------------------------------- */

std::vector<Channel*> Tracks::getChannelsIf(std::function<bool(const Channel&)> f)
{
	std::vector<Channel*> out;
	for (Track& track : m_tracks)
	{
		const std::vector<Channel*> tmp = track.getChannels().getIf(f);
		out.insert(out.end(), tmp.begin(), tmp.end());
	}
	return out;
}

/* -------------------------------------------------------------------------- */

bool Tracks::anyChannelOf(std::function<bool(const Channel&)> f) const
{
	for (const Track& track : m_tracks)
		if (track.getChannels().anyOf(f))
			return true;
	return false;
}

/* -------------------------------------------------------------------------- */

std::vector<const Channel*> Tracks::getChannels() const
{
	std::vector<const Channel*> out;
	for (const Track& track : m_tracks)
		for (const Channel& channel : track.getChannels().getAll())
			out.push_back(&channel);
	return out;
}

/* -------------------------------------------------------------------------- */

#if G_DEBUG_MODE

void Tracks::debug() const
{
	for (const Track& track : m_tracks)
		track.debug();
}

#endif

/* -------------------------------------------------------------------------- */

Track& Tracks::getByChannel(ID channelId)
{
	const auto p = [channelId](Track& track)
	{
		return u::vector::has(track.getChannels().getAll(), [channelId](const Channel& ch)
		{ return channelId == ch.id; });
	};
	return *u::vector::findIfSafe(m_tracks, p);
}

/* -------------------------------------------------------------------------- */

void Tracks::addChannel(Channel&& channel, std::size_t trackIndex)
{
	assert(channel.type != ChannelType::GROUP);
	assert(trackIndex <= m_tracks.size());

	m_tracks[trackIndex].addChannel(std::move(channel));
}

void Tracks::addChannel(Channel&& channel, std::size_t trackIndex, std::size_t position)
{
	assert(channel.type != ChannelType::GROUP);
	assert(trackIndex <= m_tracks.size());

	m_tracks[trackIndex].addChannel(std::move(channel), position);
}

/* -------------------------------------------------------------------------- */

void Tracks::removeChannel(ID channelId)
{
	getByChannel(channelId).removeChannel(channelId);
}

/* -------------------------------------------------------------------------- */

Channel& Tracks::getLastChannel(std::size_t trackIndex)
{
	assert(trackIndex <= m_tracks.size());

	return m_tracks[trackIndex].getLastChannel();
}
} // namespace giada::m::model
