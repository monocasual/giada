/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "midiEvent.h"
#include "const.h"
#include "utils/math.h"
#include <cassert>

namespace giada::m
{
MidiEvent::MidiEvent()
: m_raw(0x0)
, m_numBytes(0)
, m_delta(0)
, m_velocity(0.0f)
, m_timestamp(0)
{
}

/* -------------------------------------------------------------------------- */

MidiEvent::MidiEvent(uint32_t raw, int numBytes, double timestamp)
: m_raw(raw)
, m_numBytes(numBytes)
, m_delta(0)
, m_velocity(u::math::map(getVelocity(), G_MAX_VELOCITY, G_MAX_VELOCITY_FLOAT))
, m_timestamp(timestamp)
{
}

/* -------------------------------------------------------------------------- */

MidiEvent MidiEvent::makeFromRaw(uint32_t raw, int numBytes, double timestamp)
{
	return MidiEvent(raw, numBytes, timestamp);
}

MidiEvent MidiEvent::makeFrom3Bytes(uint8_t byte1, uint8_t byte2, uint8_t byte3, double timestamp)
{
	return MidiEvent((byte1 << 24) | (byte2 << 16) | (byte3 << 8) | 0x00, /*numBytes=*/3, timestamp);
}

MidiEvent MidiEvent::makeFrom2Bytes(uint8_t byte1, uint8_t byte2, double timestamp)
{
	return MidiEvent((byte1 << 24) | (byte2 << 16) | 0x00 | 0x00, /*numBytes=*/2, timestamp);
}

MidiEvent MidiEvent::makeFrom1Byte(uint8_t byte1, double timestamp)
{
	return MidiEvent((byte1 << 24) | 0x00 | 0x00 | 0x00, /*numBytes=*/1, timestamp);
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
	m_raw = (m_raw & ~(0x0F << 24)) | (c << 24);
}

void MidiEvent::setVelocity(int v)
{
	assert(v >= 0 && v <= G_MAX_VELOCITY);
	m_raw      = (m_raw & ~(0xFF << 8)) | (v << 8);
	m_velocity = u::math::map(v, G_MAX_VELOCITY, G_MAX_VELOCITY_FLOAT);
}

void MidiEvent::setVelocityFloat(float f)
{
	assert(f >= 0.0f && f <= G_MAX_VELOCITY_FLOAT);
	const int v = u::math::map(f, G_MAX_VELOCITY_FLOAT, G_MAX_VELOCITY);
	m_velocity  = f;
	m_raw       = (m_raw & ~(0xFF << 8)) | (v << 8);
}

/* -------------------------------------------------------------------------- */

void MidiEvent::fixVelocityZero()
{
	if (getStatus() == CHANNEL_NOTE_ON && getVelocity() == 0)
		m_raw |= CHANNEL_NOTE_OFF << 24;
}

/* -------------------------------------------------------------------------- */

MidiEvent::Type MidiEvent::getType() const
{
	/* Status byte from 0x80 to 0xE0 means musical command (CHANNEL), while
	0xF is non-musical (SYSTEM). */

	const int status = getStatus();
	if (status == 0x00)
		return Type::INVALID;
	if (status < 0xF0)
		return Type::CHANNEL;
	return Type::SYSTEM;
}

/* -------------------------------------------------------------------------- */

int MidiEvent::getStatus() const
{
	return (m_raw & 0xF0000000) >> 24;
}

int MidiEvent::getChannel() const
{
	return (m_raw & 0x0F000000) >> 24;
}

int MidiEvent::getNote() const
{
	return (m_raw & 0x00FF0000) >> 16;
}

int MidiEvent::getVelocity() const
{
	return (m_raw & 0x0000FF00) >> 8;
}

float MidiEvent::getVelocityFloat() const
{
	return m_velocity;
}

bool MidiEvent::isNoteOnOff() const
{
	return getStatus() == CHANNEL_NOTE_ON || getStatus() == CHANNEL_NOTE_OFF;
}

int MidiEvent::getDelta() const
{
	return m_delta;
}

int MidiEvent::getNumBytes() const
{
	return m_numBytes;
}

double MidiEvent::getTimestamp() const
{
	return m_timestamp;
}

int MidiEvent::getSppPosition() const
{
	assert(getType() == MidiEvent::Type::SYSTEM && getByte1() == SYSTEM_SPP);

	/* Song position: the two 7-bit data bytes (least significant byte first) 
	forming a 14-bit value which specifies the number of "MIDI beats". */

	return getByte2() | (getByte3() << 7);
}

/* -------------------------------------------------------------------------- */

uint8_t MidiEvent::getByte1() const { return (m_raw & 0xFF000000) >> 24; }
uint8_t MidiEvent::getByte2() const { return (m_raw & 0x00FF0000) >> 16; }
uint8_t MidiEvent::getByte3() const { return (m_raw & 0x0000FF00) >> 8; }

/* -------------------------------------------------------------------------- */

uint32_t MidiEvent::getRaw() const
{
	return m_raw;
}

uint32_t MidiEvent::getRawNoVelocity() const
{
	return m_raw & 0xFFFF0000;
}
} // namespace giada::m
