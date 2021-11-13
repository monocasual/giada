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

#include "core/channels/midiActionRecorder.h"
#include "core/channels/channel.h"
#include "core/conf.h"
#include "core/eventDispatcher.h"
#include "core/sequencer.h"
#include "src/core/actions/action.h"
#include "src/core/actions/actionRecorder.h"

namespace giada::m
{
MidiActionRecorder::MidiActionRecorder(ActionRecorder& a, Sequencer& s)
: m_actionRecorder(&a)
, m_sequencer(&s)
{
}

/* -------------------------------------------------------------------------- */

void MidiActionRecorder::react(Channel& ch, const EventDispatcher::Event& e, bool canRecordActions)
{
	if (e.type == EventDispatcher::EventType::MIDI && canRecordActions)
	{
		MidiEvent flat(std::get<Action>(e.data).event);
		flat.setChannel(0);
		m_actionRecorder->liveRec(ch.id, flat, m_sequencer->getCurrentFrameQuantized());
		ch.hasActions = true;
	}
}
} // namespace giada::m