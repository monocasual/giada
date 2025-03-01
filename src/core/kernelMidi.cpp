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
#include <ranges>

namespace giada::m
{
namespace
{
constexpr auto OUTPUT_NAME       = "Giada MIDI output";
constexpr auto INPUT_NAME        = "Giada MIDI input";
constexpr int  MAX_RTMIDI_EVENTS = 8;
constexpr int  MAX_NUM_PRODUCERS = 2; // Real-time thread and MIDI sync thread

/* -------------------------------------------------------------------------- */

KernelMidi::Result openPort_(RtMidi& device, int port, bool isOut)
{
	if (device.isPortOpen())
		device.closePort();

	if (port == -1)
		return {true, ""};

	const std::string deviceStr = isOut ? "out" : "in";

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

void logPorts_(RtMidi& device, std::string name)
{
	u::log::print("[KM] Device '{}': {} MIDI ports found\n", name, device.getPortCount());
	for (unsigned i = 0; i < device.getPortCount(); i++)
		u::log::print("  {}) {}\n", i, device.getPortName(i));
}

/* -------------------------------------------------------------------------- */

template <typename RtMidiType>
std::vector<std::string> getPorts_(RtMidi::Api api)
{
	std::vector<std::string>          res;
	const std::unique_ptr<RtMidiType> midiType = std::make_unique<RtMidiType>(api);

	for (unsigned int i = 0; i < midiType->getPortCount(); i++)
		res.push_back(midiType->getPortName(i));

	return res;
}

/* -------------------------------------------------------------------------- */

template <typename RtMidiType>
std::unique_ptr<RtMidiType> makeDevice_(RtMidi::Api api, std::string name)
{
	try
	{
		return std::make_unique<RtMidiType>(api, name);
	}
	catch (RtMidiError& error)
	{
		u::log::print("[KM] Error opening device '{}': {}\n", name, error.getMessage());
		return nullptr;
	}
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

template <typename RtMidiType>
KernelMidi::Device<RtMidiType>::Device(RtMidi::Api api, const std::string& name, unsigned port)
: m_port(port)
{
	try
	{
		m_rtMidi = std::make_unique<RtMidiType>(api, name);

		assert(port < m_rtMidi->getPortCount());

		G_DEBUG("***Prepare OUT device api={} name='{}' port={}", m_rtMidi->getApiName(api), name, port);
	}
	catch (const RtMidiError& error)
	{
		u::log::print("[KM] Error opening device '{}': {}\n", name, error.getMessage());
	}
}

/* -------------------------------------------------------------------------- */

template <typename RtMidiType>
bool KernelMidi::Device<RtMidiType>::isOpen() const
{
	assert(m_rtMidi != nullptr);

	return m_rtMidi->isPortOpen();
}

/* -------------------------------------------------------------------------- */

template <typename RtMidiType>
KernelMidi::Result KernelMidi::Device<RtMidiType>::open()
{
	assert(m_rtMidi != nullptr);

	try
	{
		m_rtMidi->openPort(m_port);
		u::log::print("[KM] MIDI OUT port {} opened successfully\n", m_port);
		return {true, ""};
	}
	catch (RtMidiError& error)
	{
		u::log::print("[KM] Error opening OUT port {}: {}\n", m_port, error.getMessage());
		return {false, error.getMessage()};
	}
}

/* -------------------------------------------------------------------------- */

template <typename RtMidiType>
void KernelMidi::Device<RtMidiType>::close()
{
	assert(m_rtMidi != nullptr);

	m_rtMidi->closePort();
}

/* -------------------------------------------------------------------------- */

template <typename RtMidiType>
void KernelMidi::Device<RtMidiType>::sendMessage(const RtMidiMessage& msg)
    requires std::is_same_v<RtMidiType, RtMidiOut>
{
	assert(m_rtMidi != nullptr);

	m_rtMidi->sendMessage(&msg);
}

/* -------------------------------------------------------------------------- */

template class KernelMidi::Device<RtMidiIn>;
template class KernelMidi::Device<RtMidiOut>;

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
	/* Prepare vectors of available in/out devices. */

	m_midiOuts = makeDevices<RtMidiOut>();
	// TODO - inputs

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
	if (m_midiOuts.empty())
		return;
	m_worker.start([this]()
	    {
		RtMidiMessage msg;
		while (m_midiQueue.try_dequeue(msg))
			for (auto& device : m_midiOuts)
				device.sendMessage(msg); });
}

/* -------------------------------------------------------------------------- */

bool KernelMidi::setAPI_(RtMidi::Api api)
{
	m_midiIn = makeDevice_<RtMidiIn>(api, INPUT_NAME);
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
	if (port < 0 || port >= m_midiOuts.size())
		return {false, "Invalid device"};
	return m_midiOuts[port].open();
}

KernelMidi::Result KernelMidi::openInPort_(int port)
{
	assert(m_midiIn != nullptr);

	return openPort_(*m_midiIn, port, /*isOut=*/false);
}

/* -------------------------------------------------------------------------- */

void KernelMidi::logPorts() const
{
	// TODO - log out devices
	if (m_midiIn != nullptr)
		logPorts_(*m_midiIn, INPUT_NAME);
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
	return std::ranges::any_of(m_midiOuts, [](const Device<RtMidiOut>& device)
	    { return device.isOpen(); });
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
	return getPorts_<RtMidiOut>(getAPI());
}

std::vector<std::string> KernelMidi::getInPorts() const
{
	return getPorts_<RtMidiIn>(getAPI());
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

unsigned KernelMidi::countInPorts() const { return m_midiIn != nullptr ? m_midiIn->getPortCount() : 0; }

/* -------------------------------------------------------------------------- */

template <typename RtMidiType>
std::vector<KernelMidi::Device<RtMidiType>> KernelMidi::makeDevices()
{
	std::vector<KernelMidi::Device<RtMidiType>> out;
	unsigned                                    i = 0;
	for (const std::string& portName : getPorts_<RtMidiType>(getAPI()))
		out.emplace_back(getAPI(), portName, i++);
	return out;
}

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
