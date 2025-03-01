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

#include "core/model/model.h"
#include "core/worker.h"
#include "deps/concurrentqueue/concurrentqueue.h"
#include "midiMapper.h"
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
	Data returned when opening a port. The 'message' variable will be filled
	in case of failure with the error message. */

	struct Result
	{
		bool        success = false;
		std::string message;
	};

	KernelMidi(model::Model&);

	static void logCompiledAPIs();

	/* init
	Initializes the MIDI engine given the current model::KernelMidi data. */

	bool init();

	/* setAPI
	Sets new RtMidi API. Also resets all model::KernelMidi data. */

	bool setAPI(RtMidi::Api API);

	/* open[I/0]port
	Opens a new I/O port. Also updates model::KernelMidi data if successful. */

	Result openOutPort(int port);
	Result openInPort(int port);

	/* getOutPorts, getOutPorts
	Returns a vector of port names. */

	std::vector<std::string> getOutPorts() const;
	std::vector<std::string> getInPorts() const;

	bool        hasAPI(RtMidi::Api API) const;
	RtMidi::Api getAPI() const;
	int         getSyncMode() const;
	int         getCurrentOutPort() const;
	int         getCurrentInPort() const;

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
	Starts the internal worker on a separate thread. Call this on startup. */

	void start();

	std::function<void(const MidiEvent&)> onMidiReceived;
	std::function<void()>                 onMidiSent;

private:
	using RtMidiMessage = std::vector<unsigned char>;

	template <typename RtMidiType>
	class Device
	{
	public:
		Device(RtMidi::Api, const std::string& name, unsigned port);

		bool isOpen() const;

		Result open();
		void   close();
		void   sendMessage(const RtMidiMessage&)
		    requires std::is_same_v<RtMidiType, RtMidiOut>;

	private:
		std::unique_ptr<RtMidiType> m_rtMidi;
		unsigned                    m_port;
	};

	template <typename RtMidiType>
	std::vector<Device<RtMidiType>> makeDevices();

	static void s_callback(double, RtMidiMessage*, void*);
	void        callback(double, const RtMidiMessage&);

	unsigned countInPorts() const;
	void     logPorts() const;

	bool   setAPI_(RtMidi::Api);
	Result openOutPort_(int port);
	Result openInPort_(int port);

	model::Model&                  m_model;
	std::vector<Device<RtMidiOut>> m_midiOuts;
	std::unique_ptr<RtMidiIn>      m_midiIn;

	/* m_worker
	A separate thread responsible for the MIDI output, so that multiple threads
	can access the output device simultaneously. */

	Worker m_worker;

	/* m_midiQueue
	Collects MIDI messages to be sent to the outside world. */

	mutable moodycamel::ConcurrentQueue<RtMidiMessage> m_midiQueue;

	/* m_elapsedTime
	Time elapsed on received MIDI events. Used to compute the absolute timestamp
	to pass to MidiEvent class. */

	double m_elapsedTime;
};
} // namespace giada::m

#endif
