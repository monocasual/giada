/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef G_MIDI_EVENT_H
#define G_MIDI_EVENT_H

#include <cstdint>

namespace giada::m
{
class MidiEvent
{
public:
	enum class Type
	{
		INVALID,
		CHANNEL,
		SYSTEM
	};

	/* CHANNEL_*
	List of common status bytes for Channel type. */

	static constexpr int CHANNEL_NOTE_KILL = 0x70; // Giada's special Status bye
	static constexpr int CHANNEL_NOTE_OFF  = 0x80;
	static constexpr int CHANNEL_NOTE_ON   = 0x90;
	static constexpr int CHANNEL_CC        = 0xB0; // Control Change (knobs, envelopes, ...)

	/* SYSTEM_*
	List of common status bytes for System type. */

	static constexpr int SYSTEM_SPP   = 0xF2; // Song Position Pointer
	static constexpr int SYSTEM_CLOCK = 0xF8;
	static constexpr int SYSTEM_START = 0xFA;
	static constexpr int SYSTEM_STOP  = 0xFC;

	/* MidiEvent (1)
	Creates and empty and invalid MIDI event. */

	MidiEvent();

	MidiEvent(uint32_t raw, int delta = 0);
	MidiEvent(int byte1, int byte2, int byte3, int delta = 0);

	/* MidiEvent (4)
	A constructor that takes a float parameter. Useful to build ENVELOPE events 
	for automations, volume and pitch. This will store the velocity value in
	a high-resolution float variable, instead of the limited 7-bit MIDI one.
	Use getVelocityFloat() method to retrieve it. */

	MidiEvent(float v, int delta = 0);

	Type  getType() const;
	int   getStatus() const;
	int   getChannel() const;
	int   getNote() const;
	int   getVelocity() const;
	float getVelocityFloat() const;
	bool  isNoteOnOff() const;
	int   getDelta() const;

	/* getRaw(), getRawNoVelocity()
	Returns the raw MIDI message. If getRawNoVelocity(), the velocity value is
	stripped off (i.e. velocity == 0). */

	uint32_t getRaw() const;
	uint32_t getRawNoVelocity() const;

	void setDelta(int d);
	void setChannel(int c);
	void setVelocity(int v);

	/* fixVelocityZero()
	According to the MIDI standard, there is a special case if the velocity is 
	set to zero. The NOTE ON message then has the same meaning as a NOTE OFF 
	message, switching the note off. Let's fix it. Sometime however you do want
	a NOTE ON with velocity zero: setting velocity to 0 in MIDI action editor to
	mute a specific event.  */

	void fixVelocityZero();

private:
	uint32_t m_raw;
	int      m_delta;
	float    m_velocity;
};
} // namespace giada::m

#endif
