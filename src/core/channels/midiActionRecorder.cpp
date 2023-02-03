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

#include "core/channels/midiActionRecorder.h"
#include "core/eventDispatcher.h"
#include "src/core/actions/action.h"
#include "src/core/actions/actionRecorder.h"

namespace giada::m
{
MidiActionRecorder::MidiActionRecorder(ActionRecorder& a)
: m_actionRecorder(&a)
{
}

/* -------------------------------------------------------------------------- */

void MidiActionRecorder::record(ID channelId, const MidiEvent& e, Frame currentFrameQuantized, bool& hasActions)
{
	MidiEvent flat(e);
	flat.setChannel(0);
	m_actionRecorder->liveRec(channelId, flat, currentFrameQuantized);
	hasActions = true;
}
} // namespace giada::m