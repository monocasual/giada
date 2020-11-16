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
#include "conf.h"
#include <RtMidi.h>
#include "const.h"
#include "utils/log.h"
#include "midiDispatcher.h"
#include "midiMapConf.h"
#include "kernelMidi.h"
#include "midiMsg.h"
#include "midiPorts.h"

namespace giada {
namespace m {
namespace kernelMidi
{
namespace
{

} // {anonymous}


/* -------------------------------------------------------------------------- */

void init() {

	for (std::string& p : conf::conf.midiPortInNames) {
		midiPorts::openInPort(p);
	}
	for (std::string& p : conf::conf.midiPortOutNames) {
		midiPorts::openOutPort(p);
	}
	// TODO: Notify about ports unable to open
	// TODO: Maybe try opening unavailable ports periodically?
}

/* -------------------------------------------------------------------------- */

void send(uint32_t data)
{
	// TODO: Get rid of this, it doesn't work anymore.
	std::vector msg { getB1(data), getB2(data), getB3(data) };

	midiPorts::midiReceive({"", msg}, "");

	u::log::print("[KM::send] send msg=0x%X (%X %X %X)\n", data, msg[0], msg[1], msg[2]);
}


/* -------------------------------------------------------------------------- */


void send(int b1, int b2, int b3)
{
	// TODO: Get rid of this, it doesn't work anymore.
	std::vector<unsigned char> msg(1, b1);

	if (b2 != -1)
		msg.push_back(b2);
	if (b3 != -1)
		msg.push_back(b3);

	MidiMsg mm = MidiMsg("", msg);
	midiPorts::midiReceive(mm, "");

	u::log::print("[KM::send] send msg=(%X %X %X)\n", b1, b2, b3);
}


/* -------------------------------------------------------------------------- */


void sendMidiLightning(uint32_t learnt, const midimap::Message& m)
{
	// Skip lightning message if not defined in midi map

	if (!midimap::isDefined(m))
	{
		u::log::print("[KM::sendMidiLightning] message skipped (not defined in midimap)");
		return;
	}

	u::log::print("[KM::sendMidiLightning] learnt=0x%X, chan=%d, msg=0x%X, offset=%d\n", 
		learnt, m.channel, m.value, m.offset);

	/* Isolate 'channel' from learnt message and offset it as requested by 'nn' in 
	the midimap configuration file. */

	uint32_t out = ((learnt & 0x00FF0000) >> 16) << m.offset;

	/* Merge the previously prepared channel into final message, and finally send 
	it. */

	out |= m.value | (m.channel << 24);
	send(out);
}

/* -------------------------------------------------------------------------- */


unsigned char getB1(uint32_t iValue) { return (iValue >> 24) & 0xFF; }
unsigned char getB2(uint32_t iValue) { return (iValue >> 16) & 0xFF; }
unsigned char getB3(uint32_t iValue) { return (iValue >> 8)  & 0xFF; }


}}} // giada::m::kernelMidi::
