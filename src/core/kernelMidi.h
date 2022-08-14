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

#ifndef G_KERNELMIDI_H
#define G_KERNELMIDI_H

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
	KernelMidi();

	static void logCompiledAPIs();

	unsigned countOutPorts() const;
	unsigned countInPorts() const;

	/* getOut/InPortName
    Returns the name of the port 'p'. */

	std::string getOutPortName(unsigned p) const;
	std::string getInPortName(unsigned p) const;

	bool hasAPI(RtMidi::Api API) const;

	/* send
    Sends a MIDI message to the outside world. */

	void send(const MidiEvent&);

	bool openOutDevice(RtMidi::Api api, int port);
	bool openInDevice(RtMidi::Api api, int port);

	void logPorts();

	std::function<void(const MidiEvent&)> onMidiReceived;

private:
	static void s_callback(double, std::vector<unsigned char>*, void*);
	void        callback(std::vector<unsigned char>*);

	template <typename Device>
	std::unique_ptr<Device> makeDevice(RtMidi::Api api, std::string name) const;

	std::string getPortName(RtMidi&, int port) const;
	void        logPorts(RtMidi&, std::string name) const;

	bool openPort(RtMidi&, int port);

	std::unique_ptr<RtMidiOut> m_midiOut;
	std::unique_ptr<RtMidiIn>  m_midiIn;
};
} // namespace giada::m

#endif
