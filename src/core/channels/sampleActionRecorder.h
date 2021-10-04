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

#ifndef G_CHANNEL_SAMPLE_ACTION_RECORDER_H
#define G_CHANNEL_SAMPLE_ACTION_RECORDER_H

#include "core/eventDispatcher.h"

namespace giada::m
{
class ActionRecorder;
class Sequencer;
} // namespace giada::m

namespace giada::m::channel
{
struct Data;
}

namespace giada::m::sampleActionRecorder
{
struct Data
{
	Data(ActionRecorder&, Sequencer&);

	ActionRecorder* actionRecorder;
	Sequencer*      sequencer;
};

void react(channel::Data& ch, const EventDispatcher::Event& e, bool treatRecsAsLoops,
    bool seqIsRunning, bool canRecordActions);
} // namespace giada::m::sampleActionRecorder

#endif
