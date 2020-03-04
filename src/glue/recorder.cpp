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
#include "gui/dialogs/warnings.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "core/channels/channel.h"
#include "core/const.h"
#include "core/clock.h"
#include "core/model/model.h"
#include "core/kernelMidi.h"
#include "core/recorderHandler.h"
#include "core/recorder.h"
#include "core/action.h"
#include "core/mixer.h"
#include "utils/gui.h"
#include "utils/log.h"
#include "recorder.h"


namespace giada {
namespace c {
namespace recorder 
{
void clearAllActions(ID channelId)
{
	if (!v::gdConfirmWin("Warning", "Clear all actions: are you sure?"))
		return;
	m::recorder::clearChannel(channelId);
	updateChannel(channelId, /*updateActionEditor=*/true);
}


/* -------------------------------------------------------------------------- */


void clearVolumeActions(ID channelId)
{
	if (!v::gdConfirmWin("Warning", "Clear all volume actions: are you sure?"))
		return;
	m::recorder::clearActions(channelId, m::MidiEvent::ENVELOPE);
	updateChannel(channelId, /*updateActionEditor=*/true);
}


/* -------------------------------------------------------------------------- */


void clearStartStopActions(ID channelId)
{
	if (!v::gdConfirmWin("Warning", "Clear all start/stop actions: are you sure?"))
		return;
	m::recorder::clearActions(channelId, m::MidiEvent::NOTE_ON);
	m::recorder::clearActions(channelId, m::MidiEvent::NOTE_OFF);
	m::recorder::clearActions(channelId, m::MidiEvent::NOTE_KILL);
	updateChannel(channelId, /*updateActionEditor=*/true);
}


/* -------------------------------------------------------------------------- */


void updateChannel(ID channelId, bool updateActionEditor)
{
	m::model::onGet(m::model::channels, channelId, [&](m::Channel& c)
	{
		c.state->hasActions = m::recorder::hasActions(channelId);
	});
				
	if (updateActionEditor)
		u::gui::refreshActionEditor();
}

}}} // giada::c::recorder::
