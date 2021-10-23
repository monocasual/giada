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

#include "core/kernelMidi.h"
#include "core/const.h"
#include "utils/log.h"
#include <cassert>

namespace giada::m
{
namespace
{
std::vector<unsigned char> split_(uint32_t iValue)
{
	return {
	    static_cast<unsigned char>((iValue >> 24) & 0xFF),
	    static_cast<unsigned char>((iValue >> 16) & 0xFF),
	    static_cast<unsigned char>((iValue >> 8) & 0xFF)};
}

/* -------------------------------------------------------------------------- */

uint32_t join_(int b1, int b2, int b3)
{
	return (b1 << 24) | (b2 << 16) | (b3 << 8) | (0x00);
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

KernelMidi::KernelMidi()
: onMidiReceived(nullptr)
, m_status(false)
, m_api(0)
, m_numOutPorts(0)
, m_numInPorts(0)
{
}

/* -------------------------------------------------------------------------- */

void KernelMidi::setApi(int api)
{
	m_api = api;
	u::log::print("[KM] using system 0x%x\n", m_api);
}

/* -------------------------------------------------------------------------- */

int KernelMidi::openOutDevice(int port)
{
	try
	{
		m_midiOut = std::make_unique<RtMidiOut>((RtMidi::Api)m_api, "Giada MIDI Output");
		m_status  = true;
	}
	catch (RtMidiError& error)
	{
		u::log::print("[KM] MIDI out device error: %s\n", error.getMessage());
		m_status = false;
		return 0;
	}

	/* print output ports */

	m_numOutPorts = m_midiOut->getPortCount();
	u::log::print("[KM] %d output MIDI ports found\n", m_numOutPorts);
	for (unsigned i = 0; i < m_numOutPorts; i++)
		u::log::print("  %d) %s\n", i, getOutPortName(i));

	/* try to open a port, if enabled */

	if (port != -1 && m_numOutPorts > 0)
	{
		try
		{
			m_midiOut->openPort(port, getOutPortName(port));
			u::log::print("[KM] MIDI out port %d open\n", port);
			return 1;
		}
		catch (RtMidiError& error)
		{
			u::log::print("[KM] unable to open MIDI out port %d: %s\n", port, error.getMessage());
			m_status = false;
			return 0;
		}
	}
	else
		return 2;
}

/* -------------------------------------------------------------------------- */

int KernelMidi::openInDevice(int port)
{
	try
	{
		m_midiIn = std::make_unique<RtMidiIn>((RtMidi::Api)m_api, "Giada MIDI input");
		m_status = true;
	}
	catch (RtMidiError& error)
	{
		u::log::print("[KM] MIDI in device error: %s\n", error.getMessage());
		m_status = false;
		return 0;
	}

	/* print input ports */

	m_numInPorts = m_midiIn->getPortCount();
	u::log::print("[KM] %d input MIDI ports found\n", m_numInPorts);
	for (unsigned i = 0; i < m_numInPorts; i++)
		u::log::print("  %d) %s\n", i, getInPortName(i));

	/* try to open a port, if enabled */

	if (port != -1 && m_numInPorts > 0)
	{
		try
		{
			m_midiIn->setCallback(&callback, this);
			m_midiIn->openPort(port, getInPortName(port));
			m_midiIn->ignoreTypes(true, false, true); // ignore all system/time msgs, for now
			u::log::print("[KM] MIDI in port %d open\n", port);
			return 1;
		}
		catch (RtMidiError& error)
		{
			u::log::print("[KM] unable to open MIDI in port %d: %s\n", port, error.getMessage());
			m_status = false;
			return 0;
		}
	}
	else
		return 2;
}

/* -------------------------------------------------------------------------- */

bool KernelMidi::hasAPI(int API) const
{
	std::vector<RtMidi::Api> APIs;
	RtMidi::getCompiledApi(APIs);
	for (unsigned i = 0; i < APIs.size(); i++)
		if (APIs.at(i) == API)
			return true;
	return false;
}

/* -------------------------------------------------------------------------- */

std::string KernelMidi::getOutPortName(unsigned p) const
{
	try
	{
		return m_midiOut->getPortName(p);
	}
	catch (RtMidiError& /*error*/)
	{
		return "";
	}
}

std::string KernelMidi::getInPortName(unsigned p) const
{
	try
	{
		return m_midiIn->getPortName(p);
	}
	catch (RtMidiError& /*error*/)
	{
		return "";
	}
}

/* -------------------------------------------------------------------------- */

void KernelMidi::send(uint32_t data)
{
	if (!m_status)
		return;

	std::vector<unsigned char> msg = split_(data);

	m_midiOut->sendMessage(&msg);
	u::log::print("[KM::send] send msg=0x%X (%X %X %X)\n", data, msg[0], msg[1], msg[2]);
}

/* -------------------------------------------------------------------------- */

void KernelMidi::send(int b1, int b2, int b3)
{
	if (!m_status)
		return;

	std::vector<unsigned char> msg(1, b1);

	if (b2 != -1)
		msg.push_back(b2);
	if (b3 != -1)
		msg.push_back(b3);

	m_midiOut->sendMessage(&msg);
	u::log::print("[KM::send] send msg=(%X %X %X)\n", b1, b2, b3);
}

/* -------------------------------------------------------------------------- */

unsigned KernelMidi::countInPorts() const { return m_numInPorts; }
unsigned KernelMidi::countOutPorts() const { return m_numOutPorts; }
bool     KernelMidi::getStatus() const { return m_status; }

/* -------------------------------------------------------------------------- */

void KernelMidi::callback(double /*t*/, std::vector<unsigned char>* msg, void* data)
{
	static_cast<KernelMidi*>(data)->callback(msg);
}

/* -------------------------------------------------------------------------- */

void KernelMidi::callback(std::vector<unsigned char>* msg)
{
	assert(onMidiReceived != nullptr);

	if (msg->size() < 3)
	{
		G_DEBUG("Received unknown MIDI signal - bytes=" << msg->size());
		return;
	}

	onMidiReceived(join_(msg->at(0), msg->at(1), msg->at(2)));
}
} // namespace giada::m
