/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#include "midiController.h"
#include "core/channels/channel.h"
#include "core/conf.h"
#include <cassert>

namespace giada::m::midiController
{
namespace
{
ChannelStatus onFirstBeat_(const channel::Data& ch)
{
	ChannelStatus playStatus = ch.state->playStatus.load();

	if (playStatus == ChannelStatus::ENDING)
		playStatus = ChannelStatus::OFF;
	else if (playStatus == ChannelStatus::WAIT)
		playStatus = ChannelStatus::PLAY;

	return playStatus;
}

/* -------------------------------------------------------------------------- */

ChannelStatus press_(const channel::Data& ch)
{
	ChannelStatus playStatus = ch.state->playStatus.load();

	switch (playStatus)
	{
	case ChannelStatus::PLAY:
		playStatus = ChannelStatus::ENDING;
		break;

	case ChannelStatus::ENDING:
	case ChannelStatus::WAIT:
		playStatus = ChannelStatus::OFF;
		break;

	case ChannelStatus::OFF:
		playStatus = ChannelStatus::WAIT;
		break;

	default:
		break;
	}

	return playStatus;
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void react(channel::Data& ch, const EventDispatcher::Event& e)
{
	switch (e.type)
	{

	case EventDispatcher::EventType::KEY_PRESS:
		ch.state->playStatus.store(press_(ch));
		break;

	case EventDispatcher::EventType::KEY_KILL:
	case EventDispatcher::EventType::SEQUENCER_STOP:
		ch.state->playStatus.store(ChannelStatus::OFF);
		break;

	case EventDispatcher::EventType::SEQUENCER_REWIND:
		ch.state->playStatus.store(onFirstBeat_(ch));

	default:
		break;
	}
}

/* -------------------------------------------------------------------------- */

void advance(const channel::Data& ch, const Sequencer::Event& e)
{
	if (e.type == Sequencer::EventType::FIRST_BEAT)
		ch.state->playStatus.store(onFirstBeat_(ch));
}
} // namespace giada::m::midiController
