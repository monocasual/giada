/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#include <cassert>
#include "utils/fs.h"
#include "core/channels/sampleChannel.h"
#include "core/channels/midiChannel.h"
#include "core/channels/masterChannel.h"
#include "core/channels/channel.h"
#include "core/const.h"
#include "core/patch.h"
#include "core/mixer.h"
#include "core/wave.h"
#include "core/waveManager.h"
#include "core/pluginHost.h"
#include "core/pluginManager.h"
#include "core/plugin.h"
#include "core/action.h"
#include "core/recorderHandler.h"
#include "channelManager.h"


namespace giada {
namespace m {
namespace channelManager
{
namespace
{
ID channelId_ = 0;
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


std::unique_ptr<Channel> create(ChannelType type, int bufferSize,
	bool inputMonitorOn, size_t column)
{
	std::unique_ptr<Channel> ch = nullptr;

	if (type == ChannelType::SAMPLE)
		ch = std::make_unique<SampleChannel>(inputMonitorOn, bufferSize, column, ++channelId_);
	else
	if (type == ChannelType::MIDI)
		ch = std::make_unique<MidiChannel>(bufferSize, column, ++channelId_);
	else
	if (type == ChannelType::MASTER)
		ch = std::make_unique<MasterChannel>(bufferSize, column, ++channelId_);
	
	assert(ch != nullptr);
	return ch;
}


/* -------------------------------------------------------------------------- */


std::unique_ptr<Channel> create(const Channel& o)
{
	std::unique_ptr<Channel> ch = nullptr;
	
	if (o.type == ChannelType::SAMPLE)
		ch = std::make_unique<SampleChannel>(static_cast<const SampleChannel&>(o));
	else
	if (o.type == ChannelType::MIDI)
		ch = std::make_unique<MidiChannel>(static_cast<const MidiChannel&>(o));
	else
	if (o.type == ChannelType::MASTER)
		ch = std::make_unique<MasterChannel>(static_cast<const MasterChannel&>(o));

	assert(ch != nullptr);

	if (o.type != ChannelType::MASTER)
		ch->id = ++channelId_;

	return ch;
}


/* -------------------------------------------------------------------------- */


std::unique_ptr<Channel> create(const patch::Channel& pch, int bufferSize)
{
	std::unique_ptr<Channel> ch = nullptr;

	if (static_cast<ChannelType>(pch.type) == ChannelType::SAMPLE)
		ch = std::make_unique<SampleChannel>(pch, bufferSize);
	else
	if (static_cast<ChannelType>(pch.type) == ChannelType::MIDI)
		ch = std::make_unique<MidiChannel>(pch, bufferSize);

	assert(ch != nullptr);

	channelId_ = pch.id;

	return ch;
}
}}}; // giada::m::channelManager
