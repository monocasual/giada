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


#include <vector>
#include "glue/events.h"
#include "utils/log.h"
#include "utils/math.h"
#include "core/midiDevice.h"
#include "core/midiDispatcher.h"
#include "core/midiPorts.h"
#include "core/midiMsg.h"
#include "core/model/model.h"

namespace giada {
namespace m {
namespace midiDevice
{
namespace
{

void processMaster_(const MidiEvent& midiEvent)
{
	m::model::MidiInLock l(m::model::midiIn);

	const uint32_t       pure   = midiEvent.getRawNoVelocity();
	const model::MidiIn* midiIn = model::midiIn.get();

	if      (pure == midiIn->rewind) {
		c::events::rewindSequencer(Thread::MIDI);
		u::log::print("  >>> rewind (master) (pure=0x%X)\n", pure);
	}
	else if (pure == midiIn->startStop) {
		c::events::toggleSequencer(Thread::MIDI);
		u::log::print("  >>> startStop (master) (pure=0x%X)\n", pure);
	}
	else if (pure == midiIn->actionRec) {
		c::events::toggleActionRecording();
		u::log::print("  >>> actionRec (master) (pure=0x%X)\n", pure);
	}
	else if (pure == midiIn->inputRec) {
		c::events::toggleInputRecording();
		u::log::print("  >>> inputRec (master) (pure=0x%X)\n", pure);
	}
	else if (pure == midiIn->metronome) {
		c::events::toggleMetronome();
		u::log::print("  >>> metronome (master) (pure=0x%X)\n", pure);
	}
	else if (pure == midiIn->volumeIn) {
		float vf = u::math::map(midiEvent.getVelocity(), G_MAX_VELOCITY, G_MAX_VOLUME); 
		c::events::setMasterInVolume(vf, Thread::MIDI);
		u::log::print("  >>> input volume (master) (pure=0x%X, value=%d, float=%f)\n",
			pure, midiEvent.getVelocity(), vf);
	}
	else if (pure == midiIn->volumeOut) {
		float vf = u::math::map(midiEvent.getVelocity(), G_MAX_VELOCITY, G_MAX_VOLUME); 
		c::events::setMasterOutVolume(vf, Thread::MIDI);
		u::log::print("  >>> output volume (master) (pure=0x%X, value=%d, float=%f)\n",
			pure, midiEvent.getVelocity(), vf);
	}
	else if (pure == midiIn->beatDouble) {
		c::events::multiplyBeats();
		u::log::print("  >>> sequencer x2 (master) (pure=0x%X)\n", pure);
	}
	else if (pure == midiIn->beatHalf) {
		c::events::divideBeats();
		u::log::print("  >>> sequencer /2 (master) (pure=0x%X)\n", pure);
	}
}

} // anonymous

void init() {
	midiDispatcher::registerRule(midiPorts::getInPorts(1), MMF_NOTEONOFFCC, 
			"m;midiDevice");
}

/* -------------------------------------------------------------------------- */

void midiReceive(const MidiMsg& mm)
{
	u::log::print("[MDe::midiReceive] Received message from %s.\n",
						mm.getMessageSender().c_str());
	MidiEvent midiEvent(mm.getByte(0), mm.getByte(1), mm.getByte(2));
	processMaster_(midiEvent);
}

}}} // giada::m::midiDevice::

