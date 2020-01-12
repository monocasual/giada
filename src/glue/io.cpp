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


#include <FL/Fl.H>
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/midiIO/midiInputBase.h"
#include "gui/dialogs/warnings.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/mainWindow/mainTransport.h"
#include "gui/elems/mainWindow/mainTimer.h"
#include "gui/elems/mainWindow/keyboard/keyboard.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/elems/mainWindow/keyboard/channelButton.h"
#include "gui/elems/mainWindow/keyboard/sampleChannel.h"
#include "utils/gui.h"
#include "utils/log.h"
#include "utils/math.h"
#include "core/model/model.h"
#include "core/channels/sampleChannel.h"
#include "core/channels/channel.h"
#include "core/channels/midiChannel.h"
#include "core/recorder.h"
#include "core/conf.h"
#include "core/recManager.h"
#include "core/kernelAudio.h"
#include "core/mixer.h"
#include "core/mixerHandler.h"
#include "core/wave.h"
#include "core/midiDispatcher.h"
#include "core/clock.h"
#include "core/recorderHandler.h"
#include "main.h"
#include "channel.h"
#include "io.h"


extern giada::v::gdMainWindow* G_MainWin;


namespace giada {
namespace c {
namespace io 
{
void keyPress(ID channelId, bool ctrl, bool shift, int velocity)
{
	if (ctrl)
		c::channel::toggleMute(channelId);
	else
	if (shift)
		c::channel::kill(channelId, /*record=*/true);
	else
		c::channel::start(channelId, velocity, /*record=*/true);
}


/* -------------------------------------------------------------------------- */


void keyRelease(ID channelId, bool ctrl, bool shift)
{
	if (!ctrl && !shift)
		c::channel::stop(channelId);
}


/* -------------------------------------------------------------------------- */


void setSampleChannelKey(ID channelId, int k)
{
	m::model::onSwap(m::model::channels, channelId, [&](m::Channel& c)
	{
		c.key = k;
	});

	Fl::lock();
	G_MainWin->keyboard->getChannel(channelId)->mainButton->setKey(k);
	Fl::unlock();
}


/* -------------------------------------------------------------------------- */


void midiLearn(m::MidiEvent e, std::atomic<uint32_t>& param, ID channelId)
{
	/* No MIDI learning if we are learning a Channel (channelId != 0) and 
	the selected MIDI channel is filtered OR if we are learning a global 
	parameter (channel == 0) and the selected MIDI channel is filtered. */

	if (channelId == 0) {
		if (!m::conf::isMidiInAllowed(e.getChannel()))
			return;
	}
	else {
		m::model::ChannelsLock l(m::model::channels);
		if (!m::model::get(m::model::channels, channelId).isMidiInAllowed(e.getChannel()))
			return;
	}

	param.store(e.getRawNoVelocity());
	m::midiDispatcher::stopMidiLearn();

	Fl::lock();
	u::gui::refreshSubWindow(WID_MIDI_INPUT);
	u::gui::refreshSubWindow(WID_MIDI_OUTPUT);
	Fl::unlock();
}
}}} // giada::c::io::
