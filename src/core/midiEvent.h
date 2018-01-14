/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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


namespace giada {
namespace m
{
class MidiEvent
{
public:

	static const int NOTE_ON = 0x90;
	static const int NOTE_OFF = 0x80;

	MidiEvent();
	MidiEvent(uint32_t raw);
	MidiEvent(int byte1, int byte2, int byte3);

	int getStatus() const;	
	int getChannel() const;	
	int getNote() const;	
	int getVelocity() const;	
	bool isNoteOnOff() const;	
	int getDelta() const;

	/* getRaw
	Returns the raw message. If 'velocity' is false, velocity byte is stripped
	out. */

	uint32_t getRaw(bool velocity=true) const;

	void resetDelta();
	void setChannel(int c);

private:

	uint32_t m_raw;
	int m_status;
	int m_channel;
	int m_note;
	int m_velocity;
	int m_delta;
};

}} // giada::m::


#endif