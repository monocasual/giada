/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/core/model/channel.h"
#include "src/core/rendering/midiReactions.h"
#include "src/core/actions/ActionManager.h"
#include "src/core/kernelMidi.h"
#include <cassert>

namespace giada::m::rendering
{
void playMidiChannel(const model::Channel::Parameters& params)
{
	ChannelStatus playStatus = params.getPlayStatus();

	switch (playStatus)
	{
	case ChannelStatus::PLAY:
		playStatus = ChannelStatus::ENDING;
		break;

	case ChannelStatus::ENDING:
		playStatus = ChannelStatus::PLAY;
		break;

	case ChannelStatus::WAIT:
		playStatus = ChannelStatus::OFF;
		break;

	case ChannelStatus::OFF:
		playStatus = ChannelStatus::WAIT;
		break;

	default:
		break;
	}

	params.setPlayStatus(playStatus);
}


/* -------------------------------------------------------------------------- */

void stopMidiChannel(const model::Channel::Parameters& params)
{
	params.setPlayStatus(ChannelStatus::OFF);
}


/* -------------------------------------------------------------------------- */

void rewindMidiChannel(const model::Channel::Parameters& params)
{
	ChannelStatus playStatus = params.getPlayStatus();

	if (playStatus == ChannelStatus::ENDING)
		playStatus = ChannelStatus::OFF;
	else if (playStatus == ChannelStatus::WAIT)
		playStatus = ChannelStatus::PLAY;

	params.setPlayStatus(playStatus);
}

/* -------------------------------------------------------------------------- */

void recordMidiAction(ID channelId, Scene scene, const MidiEvent& event,
    Tick currentTickQuantized, ActionManager& actionRecorder)
{
	MidiEvent flat(event);
	flat.setChannel(0);
	actionRecorder.liveRec(channelId, scene, flat, currentTickQuantized);
}
} // namespace giada::m::rendering
