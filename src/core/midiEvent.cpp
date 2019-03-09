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
#include "const.h"
#include "midiEvent.h"


namespace giada {
namespace m
{
MidiEvent::MidiEvent()
	: m_status  (0),
	  m_channel (0),
	  m_note    (0),
	  m_velocity(0),
	  m_delta   (0)
{
}


/* -------------------------------------------------------------------------- */


MidiEvent::MidiEvent(uint32_t raw)
	: m_status  ((raw & 0xF0000000) >> 24),
	  m_channel ((raw & 0x0F000000) >> 24),
	  m_note    ((raw & 0x00FF0000) >> 16),
	  m_velocity((raw & 0x0000FF00) >> 8),
	  m_delta   (0)  // not used
{
}


/* -------------------------------------------------------------------------- */


MidiEvent::MidiEvent(int byte1, int byte2, int byte3)
	: MidiEvent((byte1 << 24) | (byte2 << 16) | (byte3 << 8) | (0x00))
{
}


/* -------------------------------------------------------------------------- */


void MidiEvent::setDelta(int d)
{
	m_delta = d;
}


/* -------------------------------------------------------------------------- */


void MidiEvent::setChannel(int c)
{
	assert(c >= 0 && c < G_MAX_MIDI_CHANS);
	m_channel = c;
}


void MidiEvent::setVelocity(int v)
{
	assert(v >= 0 && v <= G_MAX_VELOCITY);
	m_velocity = v;
}


/* -------------------------------------------------------------------------- */


void MidiEvent::fixVelocityZero()
{
	if (m_status == NOTE_ON && m_velocity == 0)
		m_status = NOTE_OFF;
}


/* -------------------------------------------------------------------------- */


int MidiEvent::getStatus() const
{
	return m_status;
}	


int MidiEvent::getChannel() const
{
	return m_channel;
}


int MidiEvent::getNote() const
{
	return m_note;
}	


int MidiEvent::getVelocity() const
{
	return m_velocity;
}	


bool MidiEvent::isNoteOnOff() const
{
	return m_status == NOTE_ON || m_status == NOTE_OFF;
}


int MidiEvent::getDelta() const
{
	return m_delta;
}


/* -------------------------------------------------------------------------- */


uint32_t MidiEvent::getRaw() const
{
	return (m_status << 24) | (m_channel << 24) | (m_note << 16) | (m_velocity << 8) | (0x00);
}


uint32_t MidiEvent::getRawNoVelocity() const
{
	return (m_status << 24) | (m_channel << 24) | (m_note << 16) | (0x00 << 8) | (0x00);
}


}} // giada::m::
