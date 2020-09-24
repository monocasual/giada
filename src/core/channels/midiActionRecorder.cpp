/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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
#include "core/action.h"
#include "core/clock.h"
#include "core/conf.h"
#include "core/mixer.h"
#include "core/recorderHandler.h"
#include "core/recManager.h"
#include "core/channels/state.h"
#include "midiActionRecorder.h"


namespace giada {
namespace m
{
MidiActionRecorder::MidiActionRecorder(ChannelState* c)
: m_channelState(c)
{
}


/* -------------------------------------------------------------------------- */


MidiActionRecorder::MidiActionRecorder(const MidiActionRecorder& /*o*/, ChannelState* c)
: MidiActionRecorder(c)
{
}


/* -------------------------------------------------------------------------- */


void MidiActionRecorder::parse(const mixer::Event& e) const
{
	assert(m_channelState != nullptr);

	if (e.type == mixer::EventType::MIDI && canRecord()) 
		record(e.action.event);
}


/* -------------------------------------------------------------------------- */


void MidiActionRecorder::record(const MidiEvent& e) const
{
	MidiEvent flat(e);
	flat.setChannel(0);
	recorderHandler::liveRec(m_channelState->id, flat, clock::quantize(clock::getCurrentFrame()));
	m_channelState->hasActions = true;
}


/* -------------------------------------------------------------------------- */


bool MidiActionRecorder::canRecord() const
{
	return recManager::isRecordingAction() && 
	       clock::isRunning()              && 
	       !recManager::isRecordingInput();
}
}} // giada::m::

