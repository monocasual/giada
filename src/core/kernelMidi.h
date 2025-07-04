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

#ifndef G_KERNELMIDI_H
#define G_KERNELMIDI_H

#include "src/core/midiMapper.h"
#include "src/core/model/model.h"
#include "src/core/worker.h"
#include "src/deps/concurrentqueue/concurrentqueue.h"
#include <RtMidi.h>
#include <concepts>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>

namespace giada::m
{
class MidiEvent;
class KernelMidi final
{
public:
	/* Result
	Data returned when opening a device. The 'message' variable will be filled
	in case of failure with the error message. */

	struct Result
	{
		bool        success = false;
		std::string message;
	};

	/* DeviceInfo
	Information about an input/output device. */

	struct DeviceInfo
	{
		std::size_t index;
		std::string name;
		bool        isOpen;
	};

	KernelMidi(model::Model&);

	static void logCompiledAPIs();

	/* init
	Initializes the MIDI engine given the current model::KernelMidi data. */

	bool init();

	/* setAPI
	Sets new RtMidi API. Also resets all model::KernelMidi data. */

	bool setAPI(RtMidi::Api API);

	/* [open|close][I/0]device
	Opens or closes a new I/O device. Also updates model::KernelMidi data if
	successful. */

	Result openOutDevice(std::size_t deviceIndex);
	Result openInDevice(std::size_t deviceIndex);
	void   closeOutDevice(std::size_t deviceIndex);
	void   closeInDevice(std::size_t deviceIndex);

	/* getAvailable[Out|In]Devices
	Return vectors of device information. Use vector indexes coming from each
	DeviceInfo object to specify which devices to open. */

	std::vector<DeviceInfo> getAvailableOutDevices() const;
	std::vector<DeviceInfo> getAvailableInDevices() const;

	bool        hasAPI(RtMidi::Api API) const;
	RtMidi::Api getAPI() const;
	int         getSyncMode() const;

	/* canSend, canReceive
	Return true if KernelMidi is capable of sending/receiving MIDI messages,
	given the current configuration. */

	bool canSend() const;
	bool canReceive() const;

	/* canSyncMaster, canSyncSlave
	Return true if KernelMidi has MIDI sync enabled, master or slave, given the
	current configuration. */

	bool canSyncMaster() const;
	bool canSyncSlave() const;

	/* send
	Sends a MIDI message to the outside world. Returns false if MIDI out is not
	enabled or the internal queue is full. */

	bool send(const MidiEvent&) const;

	/* start
	Starts the internal workers on separate threads. Call this on startup. */

	void start();

	std::function<void(const MidiEvent&)> onMidiReceived;
	std::function<void()>                 onMidiSent;

private:
	using RtMidiMessage = std::vector<unsigned char>;

	template <typename RtMidiType>
	class Device
	{
	public:
		Device(RtMidi::Api, const std::string& name, unsigned port, KernelMidi&);

		bool        isOpen() const;
		std::string getName() const;

		Result open();
		void   close();
		void   sendMessage(const RtMidiMessage&)
		    requires std::is_same_v<RtMidiType, RtMidiOut>;

	private:
		static void s_callback(double, RtMidiMessage*, void*)
		    requires std::is_same_v<RtMidiType, RtMidiIn>;
		void callback(double, const RtMidiMessage&)
		    requires std::is_same_v<RtMidiType, RtMidiIn>;

		std::string getTypeStr() const;

		std::unique_ptr<RtMidiType> m_rtMidi;
		unsigned                    m_port;
		KernelMidi&                 m_kernelMidi;

		/* m_elapsedTime
		Time elapsed on received MIDI events. Used only with RtMidiIn type to
		compute the absolute timestamp to pass to MidiEvent class. */

		double m_elapsedTime;
	};

	/* Devices
	Vector of KernelMidi::Device objects. Not a simple std::vector<Device>, but
	rather a vector of unique_ptrs. This is necessary because we are passing
	pointers to Devices around callbacks (for MIDI in), so all pointers to Devices
	must remain valid during copy/move operations. */

	template <typename RtMidiType>
	using Devices = std::vector<std::unique_ptr<Device<RtMidiType>>>;

	template <typename RtMidiType>
	Devices<RtMidiType> makeDevices();

	template <typename RtMidiType>
	std::vector<DeviceInfo> getDevicesInfo(const Devices<RtMidiType>&) const;

	template <typename RtMidiType>
	void openDevices(Devices<RtMidiType>&, const std::set<std::size_t>);

	Result openOutDevice_(std::size_t deviceIndex);
	Result openInDevice_(std::size_t deviceIndex);

	model::Model&      m_model;
	Devices<RtMidiOut> m_midiOuts;
	Devices<RtMidiIn>  m_midiIns;

	/* m_outputWorker
	A separate thread responsible for the MIDI output, so that multiple threads
	can access the output device simultaneously. */

	Worker m_outputWorker;

	/* m_inputWorker
	A separate thread responsible for the MIDI input. It pops MIDI events from
	the inputQueue and notify listeners via onMidiReceived callback. */

	Worker m_inputWorker;

	/* m_outputQueue
	Collects MIDI messages to be sent to the outside world. */

	mutable moodycamel::ConcurrentQueue<RtMidiMessage> m_outputQueue;

	/* m_inputQueue
	Collects MIDI events received from the outside world from multiple threads
	(devices). */

	mutable moodycamel::ConcurrentQueue<MidiEvent> m_inputQueue;
};
} // namespace giada::m

#endif
