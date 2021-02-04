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


#include <cassert>
#include "core/conf.h"
#include "core/channels/state.h"
#include "midiController.h"


namespace giada {
namespace m 
{
MidiController::MidiController(ChannelState* c)
: m_channelState(c)
{
}


/* -------------------------------------------------------------------------- */


MidiController::MidiController(const MidiController& /*o*/, ChannelState* c)
: m_channelState(c)
{
}


/* -------------------------------------------------------------------------- */


void MidiController::parse(const mixer::Event& e) const
{
	assert(m_channelState != nullptr);

	switch (e.type) {

		case mixer::EventType::KEY_PRESS:
			press(); break;

		case mixer::EventType::KEY_KILL:
		case mixer::EventType::SEQUENCER_STOP:
			kill();	break;

		case mixer::EventType::SEQUENCER_FIRST_BEAT:
		case mixer::EventType::SEQUENCER_REWIND:
			onFirstBeat();	

		default: break;
	}
}


/* -------------------------------------------------------------------------- */


void MidiController::press() const
{
    ChannelStatus playStatus = m_channelState->playStatus.load();

	switch (playStatus) {
		case ChannelStatus::PLAY:
			playStatus = ChannelStatus::ENDING; break;

		case ChannelStatus::ENDING:
		case ChannelStatus::WAIT:
			playStatus = ChannelStatus::OFF; break;

		case ChannelStatus::OFF:
			playStatus = ChannelStatus::WAIT; break;

		default: break;
	}	
	
	m_channelState->playStatus.store(playStatus);
}


/* -------------------------------------------------------------------------- */


void MidiController::kill() const
{
	m_channelState->playStatus.store(ChannelStatus::OFF);
}


/* -------------------------------------------------------------------------- */


void MidiController::onFirstBeat() const
{
	ChannelStatus playStatus = m_channelState->playStatus.load();

	if (playStatus == ChannelStatus::ENDING)
		playStatus = ChannelStatus::OFF;
	else
	if (playStatus == ChannelStatus::WAIT)
		playStatus = ChannelStatus::PLAY;
	
	m_channelState->playStatus.store(playStatus);
}
}} // giada::m::
