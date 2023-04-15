/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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
	KernelMidi(model::Model&);

	static void logCompiledAPIs();

	/* init
	Initializes the MIDI engine given the current model::KernelMidi data. */

	bool init();

	/* getOutPorts, getOutPorts
    Returns a vector of port names. */

	std::vector<std::string> getOutPorts() const;
	std::vector<std::string> getInPorts() const;

	bool        hasAPI(RtMidi::Api API) const;
	RtMidi::Api getAPI() const;
	int         getSyncMode() const;
	int         getCurrentOutPort() const;
	int         getCurrentInPort() const;

	/* send
    Sends a MIDI message to the outside world. Returns false if MIDI out is not
	enabled or the internal queue is full. */

	bool send(const MidiEvent&) const;

	/* start
	Starts the internal worker on a separate thread. Call this on startup. */

	void start();

	void logPorts();

	std::function<void(const MidiEvent&)> onMidiReceived;
	std::function<void()>                 onMidiSent;

private:
	using RtMidiMessage = std::vector<unsigned char>;

	static void s_callback(double, RtMidiMessage*, void*);
	void        callback(double, RtMidiMessage*);

	template <typename Device>
	std::unique_ptr<Device> makeDevice(RtMidi::Api api, std::string name) const;

	unsigned    countOutPorts() const;
	unsigned    countInPorts() const;
	std::string getPortName(RtMidi&, int port) const;
	void        logPorts(RtMidi&, std::string name) const;

	bool openOutDevice(RtMidi::Api api, int port);
	bool openInDevice(RtMidi::Api api, int port);
	bool openPort(RtMidi&, int port);

	model::Model&              m_model;
	std::unique_ptr<RtMidiOut> m_midiOut;
	std::unique_ptr<RtMidiIn>  m_midiIn;

	/* m_worker
	A separate thread responsible for the MIDI output, so that multiple threads
	can access the output device simultaneously. */

	Worker m_worker;

	/* m_midiQueue
	Collects MIDI messages to be sent to the outside world. */

	mutable moodycamel::ConcurrentQueue<RtMidiMessage> m_midiQueue;

	/* m_elpsedTime
	Time elapsed on received MIDI events. Used to compute the absolute timestamp
	to pass to MidiEvent class. */

	double m_elpsedTime;
};
} // namespace giada::m

#endif
