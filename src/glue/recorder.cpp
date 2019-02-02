/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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
#include "../gui/dialogs/warnings.h"
#include "../gui/elems/mainWindow/keyboard/channel.h"
#include "../gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "../core/const.h"
#include "../core/clock.h"
#include "../core/kernelMidi.h"
#include "../core/channel.h"
#include "../core/recorderHandler.h"
#include "../core/recorder.h"
#include "../core/action.h"
#include "../core/mixer.h"
#include "../core/sampleChannel.h"
#include "../core/midiChannel.h"
#include "../utils/gui.h"
#include "../utils/log.h"
#include "recorder.h"


using std::vector;
using namespace giada;


namespace giada {
namespace c {
namespace recorder 
{
void clearAllActions(geChannel* gch)
{
	if (!gdConfirmWin("Warning", "Clear all actions: are you sure?"))
		return;
	gch->ch->kill(0);
	m::recorder::clearChannel(gch->ch->index);
	updateChannel(gch);
}


/* -------------------------------------------------------------------------- */


void clearVolumeActions(geChannel* gch)
{
	if (!gdConfirmWin("Warning", "Clear all volume actions: are you sure?"))
		return;
	m::recorder::clearActions(gch->ch->index, m::MidiEvent::ENVELOPE);
	updateChannel(gch);
}


/* -------------------------------------------------------------------------- */


void clearStartStopActions(geChannel* gch)
{
	if (!gdConfirmWin("Warning", "Clear all start/stop actions: are you sure?"))
		return;
	gch->ch->kill(0);
	m::recorder::clearActions(gch->ch->index, m::MidiEvent::NOTE_ON);
	m::recorder::clearActions(gch->ch->index, m::MidiEvent::NOTE_OFF);
	m::recorder::clearActions(gch->ch->index, m::MidiEvent::NOTE_KILL);
	updateChannel(gch);
}


/* -------------------------------------------------------------------------- */


void updateChannel(geChannel* gch, bool refreshActionEditor)
{
	gch->ch->hasActions = m::recorder::hasActions(gch->ch->index);
	if (!gch->ch->hasActions)
		gch->ch->readActions = false;

	if (gch->ch->type == ChannelType::SAMPLE) {
		geSampleChannel* gsch = static_cast<geSampleChannel*>(gch);
		gsch->ch->hasActions ? gsch->showActionButton() : gsch->hideActionButton();
	}
	if (refreshActionEditor)
		gu_refreshActionEditor(); // refresh a.editor window, it could be open
}

}}} // giada::c::recorder::