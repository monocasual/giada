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
#include "core/eventDispatcher.h"
#include "core/action.h"
#include "core/clock.h"
#include "core/conf.h"
#include "core/mixer.h"
#include "core/recorderHandler.h"
#include "core/recManager.h"
#include "core/channels/channel.h"
#include "midiActionRecorder.h"


namespace giada::m::midiActionRecorder
{
namespace
{
void record_(channel::Data& ch, const MidiEvent& e)
{
	MidiEvent flat(e);
	flat.setChannel(0);
	recorderHandler::liveRec(ch.id, flat, clock::quantize(clock::getCurrentFrame()));
	ch.hasActions = true;
}


/* -------------------------------------------------------------------------- */


bool canRecord_()
{
	return recManager::isRecordingAction() && 
	       clock::isRunning()              && 
	       !recManager::isRecordingInput();
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void react(channel::Data& ch, const eventDispatcher::Event& e)
{
	if (e.type == eventDispatcher::EventType::MIDI && canRecord_())
		record_(ch, std::get<Action>(e.data).event);
}
} // giada::m::midiActionRecorder::