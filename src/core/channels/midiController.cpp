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

#include "midiController.h"

namespace giada::m
{
void MidiController::advance(WeakAtomic<ChannelStatus>& a_playStatus, const Sequencer::Event& e) const
{
	if (e.type == Sequencer::EventType::FIRST_BEAT)
		rewind(a_playStatus);
}

/* -------------------------------------------------------------------------- */

void MidiController::keyPress(WeakAtomic<ChannelStatus>& a_playStatus) const
{
	a_playStatus.store(press(a_playStatus.load()));
}

/* -------------------------------------------------------------------------- */

void MidiController::keyKill(WeakAtomic<ChannelStatus>& a_playStatus) const
{
	stop(a_playStatus);
}

/* -------------------------------------------------------------------------- */

void MidiController::stop(WeakAtomic<ChannelStatus>& a_playStatus) const
{
	a_playStatus.store(ChannelStatus::OFF);
}

/* -------------------------------------------------------------------------- */

void MidiController::rewind(WeakAtomic<ChannelStatus>& a_playStatus) const
{
	a_playStatus.store(onFirstBeat(a_playStatus.load()));
}

/* -------------------------------------------------------------------------- */

ChannelStatus MidiController::onFirstBeat(ChannelStatus playStatus) const
{
	if (playStatus == ChannelStatus::ENDING)
		playStatus = ChannelStatus::OFF;
	else if (playStatus == ChannelStatus::WAIT)
		playStatus = ChannelStatus::PLAY;

	return playStatus;
}

/* -------------------------------------------------------------------------- */

ChannelStatus MidiController::press(ChannelStatus playStatus) const
{
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

	return playStatus;
}
} // namespace giada::m
