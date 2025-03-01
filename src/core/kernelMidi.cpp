/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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
#include "core/midiEvent.h"
#include "core/model/kernelAudio.h"
#include "utils/log.h"
#include <algorithm>
#include <cassert>
#include <chrono>
#include <memory>

namespace giada::m
{
namespace
{
constexpr auto OUTPUT_NAME       = "Giada MIDI output";
constexpr auto INPUT_NAME        = "Giada MIDI input";
constexpr int  MAX_RTMIDI_EVENTS = 8;
constexpr int  MAX_NUM_PRODUCERS = 2; // Real-time thread and MIDI sync thread
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

KernelMidi::KernelMidi(model::Model& m)
: onMidiReceived(nullptr)
, onMidiSent(nullptr)
, m_model(m)
, m_worker(G_KERNEL_MIDI_OUTPUT_RATE_MS)
, m_midiQueue(MAX_RTMIDI_EVENTS, 0, MAX_NUM_PRODUCERS) // See https://github.com/cameron314/concurrentqueue#preallocation-correctly-using-try_enqueue
, m_elapsedTime(0.0)
{
}

/* -------------------------------------------------------------------------- */

bool KernelMidi::init()
{
	const model::KernelMidi& kernelMidi = m_model.get().kernelMidi;

	if (!setAPI_(kernelMidi.api))
		return false;
	if (!openOutPort_(kernelMidi.portOut).success)
		return false;
	if (!openInPort_(kernelMidi.portIn).success)
		return false;

	return true;
}

/* -------------------------------------------------------------------------- */

bool KernelMidi::setAPI(RtMidi::Api api)
{
	if (!setAPI_(api))
		return false;

	m_model.get().kernelMidi.api     = api;
	m_model.get().kernelMidi.portOut = G_DEFAULT_MIDI_PORT_OUT;
	m_model.get().kernelMidi.portIn  = G_DEFAULT_MIDI_PORT_IN;
	m_model.swap(model::SwapType::NONE);

	return true;
}

/* -------------------------------------------------------------------------- */

KernelMidi::Result KernelMidi::openOutPort(int port)
{
	Result res = openOutPort_(port);

	if (!res.success)
		return res;

	m_model.get().kernelMidi.portOut = port;
	m_model.swap(model::SwapType::NONE);

	return res;
}

KernelMidi::Result KernelMidi::openInPort(int port)
{
	Result res = openInPort_(port);

	if (!res.success)
		return res;

	m_model.get().kernelMidi.portIn = port;
	m_model.swap(model::SwapType::NONE);

	return res;
}

/* -------------------------------------------------------------------------- */

void KernelMidi::start()
{
	if (m_midiOut == nullptr)
		return;
	m_worker.start([this]()
	{
		RtMidiMessage msg;
		while (m_midiQueue.try_dequeue(msg))
			m_midiOut->sendMessage(&msg);
	});
}

/* -------------------------------------------------------------------------- */

bool KernelMidi::setAPI_(RtMidi::Api api)
{
	m_midiOut = makeDevice<RtMidiOut>(api, OUTPUT_NAME);
	m_midiIn  = makeDevice<RtMidiIn>(api, INPUT_NAME);

	if (m_midiIn == nullptr || m_midiOut == nullptr)
		return false;

	if (m_midiIn != nullptr)
	{
		m_midiIn->setCallback(&s_callback, this);
		m_midiIn->ignoreTypes(/*midiSysex=*/true, /*midiTime=*/false, /*midiSense=*/true); // Don't ignore time msgs
	}

	logPorts();

	return true;
}

/* -------------------------------------------------------------------------- */

KernelMidi::Result KernelMidi::openOutPort_(int port)
{
	assert(m_midiOut != nullptr);

	return openPort(*m_midiOut, port);
}

KernelMidi::Result KernelMidi::openInPort_(int port)
{
	assert(m_midiIn != nullptr);

	return openPort(*m_midiIn, port);
}

/* -------------------------------------------------------------------------- */

void KernelMidi::logPorts() const
{
	if (m_midiOut != nullptr)
		logPorts(*m_midiOut, OUTPUT_NAME);
	if (m_midiIn != nullptr)
		logPorts(*m_midiIn, INPUT_NAME);
}

/* -------------------------------------------------------------------------- */

bool KernelMidi::hasAPI(RtMidi::Api API) const
{
	std::vector<RtMidi::Api> APIs;
	RtMidi::getCompiledApi(APIs);
	for (unsigned i = 0; i < APIs.size(); i++)
		if (APIs.at(i) == API)
			return true;
	return false;
}

/* -------------------------------------------------------------------------- */

RtMidi::Api KernelMidi::getAPI() const { return m_model.get().kernelMidi.api; }
int         KernelMidi::getSyncMode() const { return m_model.get().kernelMidi.sync; }
int         KernelMidi::getCurrentOutPort() const { return m_model.get().kernelMidi.portOut; }
int         KernelMidi::getCurrentInPort() const { return m_model.get().kernelMidi.portIn; }

/* -------------------------------------------------------------------------- */

bool KernelMidi::canSend() const
{
	return m_midiOut && m_midiOut->isPortOpen();
}

bool KernelMidi::canReceive() const
{
	return m_midiIn && m_midiIn->isPortOpen();
}

/* -------------------------------------------------------------------------- */

bool KernelMidi::canSyncMaster() const
{
	return canSend() && m_model.get().kernelMidi.sync == G_MIDI_SYNC_CLOCK_MASTER;
}

bool KernelMidi::canSyncSlave() const
{
	return canReceive() && m_model.get().kernelMidi.sync == G_MIDI_SYNC_CLOCK_SLAVE;
}

/* -------------------------------------------------------------------------- */

std::vector<std::string> KernelMidi::getOutPorts() const
{
	std::vector<std::string> out;
	for (unsigned i = 0; i < countOutPorts(); i++)
		out.push_back(getPortName(*m_midiOut, i));
	return out;
}

std::vector<std::string> KernelMidi::getInPorts() const
{
	std::vector<std::string> out;
	for (unsigned i = 0; i < countInPorts(); i++)
		out.push_back(getPortName(*m_midiIn, i));
	return out;
}

/* -------------------------------------------------------------------------- */

bool KernelMidi::send(const MidiEvent& event) const
{
	if (!canSend())
		return false;

	assert(event.getNumBytes() > 0 && event.getNumBytes() <= 3);
	assert(onMidiSent != nullptr);

	RtMidiMessage msg;
	if (event.getNumBytes() == 1)
		msg = {event.getByte1()};
	else if (event.getNumBytes() == 2)
		msg = {event.getByte1(), event.getByte2()};
	else
		msg = {event.getByte1(), event.getByte2(), event.getByte3()};

	G_DEBUG("Send MIDI msg=0x{:0X}", event.getRaw());

	onMidiSent();

	return m_midiQueue.try_enqueue(msg);
}

/* -------------------------------------------------------------------------- */

unsigned KernelMidi::countOutPorts() const { return m_midiOut != nullptr ? m_midiOut->getPortCount() : 0; }
unsigned KernelMidi::countInPorts() const { return m_midiIn != nullptr ? m_midiIn->getPortCount() : 0; }

/* -------------------------------------------------------------------------- */

void KernelMidi::s_callback(double deltatime, RtMidiMessage* msg, void* data)
{
	static_cast<KernelMidi*>(data)->callback(deltatime, *msg);
}

/* -------------------------------------------------------------------------- */

void KernelMidi::callback(double deltatime, const RtMidiMessage& msg)
{
	assert(onMidiReceived != nullptr);
	assert(msg.size() > 0);

	m_elapsedTime += deltatime;

	MidiEvent event;
	if (msg.size() == 1)
		event = MidiEvent::makeFrom1Byte(msg[0], m_elapsedTime);
	else if (msg.size() == 2)
		event = MidiEvent::makeFrom2Bytes(msg[0], msg[1], m_elapsedTime);
	else if (msg.size() == 3)
		event = MidiEvent::makeFrom3Bytes(msg[0], msg[1], msg[2], m_elapsedTime);
	else
		assert(false); // MIDI messages longer than 3 bytes are not supported

	onMidiReceived(event);

	G_DEBUG("Recv MIDI msg=0x{:0X}, timestamp={}", event.getRaw(), m_elapsedTime);
}

/* -------------------------------------------------------------------------- */

template <typename Device>
std::unique_ptr<Device> KernelMidi::makeDevice(RtMidi::Api api, std::string name) const
{
	try
	{
		return std::make_unique<Device>(static_cast<RtMidi::Api>(api), name);
	}
	catch (RtMidiError& error)
	{
		u::log::print("[KM] Error opening device '{}': {}\n", name, error.getMessage());
		return nullptr;
	}
}

template std::unique_ptr<RtMidiOut> KernelMidi::makeDevice(RtMidi::Api, std::string) const;
template std::unique_ptr<RtMidiIn>  KernelMidi::makeDevice(RtMidi::Api, std::string) const;

/* -------------------------------------------------------------------------- */

KernelMidi::Result KernelMidi::openPort(RtMidi& device, int port)
{
	if (device.isPortOpen())
		device.closePort();

	if (port == -1)
		return {true, ""};

	const std::string deviceStr = &device == m_midiOut.get() ? "out" : "in";

	try
	{
		device.openPort(port, device.getPortName(port));
		u::log::print("[KM] MIDI {} port {} opened successfully\n", deviceStr, port);
		return {true, ""};
	}
	catch (RtMidiError& error)
	{
		u::log::print("[KM] Error opening {} port {}: {}\n", deviceStr, port, error.getMessage());
		return {false, error.getMessage()};
	}
}

/* -------------------------------------------------------------------------- */

std::string KernelMidi::getPortName(RtMidi& device, int port) const
{
	try
	{
		return device.getPortName(port);
	}
	catch (RtMidiError& /*error*/)
	{
		return "";
	}
}

/* -------------------------------------------------------------------------- */

void KernelMidi::logPorts(RtMidi& device, std::string name) const
{
	u::log::print("[KM] Device '{}': {} MIDI ports found\n", name, device.getPortCount());
	for (unsigned i = 0; i < device.getPortCount(); i++)
		u::log::print("  {}) {}\n", i, device.getPortName(i));
}

/* -------------------------------------------------------------------------- */

void KernelMidi::logCompiledAPIs()
{
	std::vector<RtMidi::Api> apis;
	RtMidi::getCompiledApi(apis);

	u::log::print("[KM] Compiled RtMidi APIs: {}\n", apis.size());

	for (const RtMidi::Api& api : apis)
	{
		switch (api)
		{
		case RtMidi::Api::UNSPECIFIED:
			u::log::print("  UNSPECIFIED\n");
			break;
		case RtMidi::Api::MACOSX_CORE:
			u::log::print("  CoreAudio\n");
			break;
		case RtMidi::Api::LINUX_ALSA:
			u::log::print("  ALSA\n");
			break;
		case RtMidi::Api::UNIX_JACK:
			u::log::print("  JACK\n");
			break;
		case RtMidi::Api::WINDOWS_MM:
			u::log::print("  Microsoft Multimedia MIDI API\n");
			break;
		case RtMidi::Api::RTMIDI_DUMMY:
			u::log::print("  Dummy\n");
			break;
		case RtMidi::Api::WEB_MIDI_API:
			u::log::print("  Web MIDI API\n");
			break;
		default:
			u::log::print("  (unknown)\n");
			break;
		}
	}
}
} // namespace giada::m
