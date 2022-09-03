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

#ifndef G_MIDI_DISPATCHER_H
#define G_MIDI_DISPATCHER_H

#include "core/actions/action.h"
#include "core/midiEvent.h"
#include "core/model/model.h"
#include "core/types.h"
#include <cstddef>
#include <cstdint>
#include <functional>

namespace giada::m
{
class MidiDispatcher
{
public:
	MidiDispatcher(model::Model&);

	void startChannelLearn(int param, ID channelId, std::function<void()> f);
	void startMasterLearn(int param, std::function<void()> f);
	void stopLearn();
	void clearMasterLearn(int param, std::function<void()> f);
	void clearChannelLearn(int param, ID channelId, std::function<void()> f);
	void startPluginLearn(std::size_t paramIndex, ID pluginId, std::function<void()> f);
	void clearPluginLearn(std::size_t paramIndex, ID pluginId, std::function<void()> f);

	/* dispatch
    Main callback invoked by kernelMidi whenever a new MIDI data comes in. */

	void dispatch(const MidiEvent&);

	/* onEventReceived
	Callback fired when a MIDI event of type CHANNEL has been received. */

	std::function<void()> onEventReceived;

private:
	/* learn
    Learns event 'e'. Called by the Event Dispatcher. */

	void learn(const MidiEvent&);

	/* process
    Sends event 'e' to channels (masters and keyboard). Called by the Event 
    Dispatcher. */

	void process(const MidiEvent&);

	bool isMasterMidiInAllowed(int c);
	bool isChannelMidiInAllowed(ID channelId, int c);

	void processChannels(const MidiEvent&);
	void processMaster(const MidiEvent&);

	void learnChannel(MidiEvent, int param, ID channelId, std::function<void()> doneCb);
	void learnMaster(MidiEvent, int param, std::function<void()> doneCb);

	void processPlugins(ID channelId, const std::vector<Plugin*>& plugins, const MidiEvent&);
	void learnPlugin(MidiEvent, std::size_t paramIndex, ID pluginId, std::function<void()> doneCb);

	/* cb_midiLearn
    Callback prepared by the gdMidiGrabber window and called by midiDispatcher. 
    It contains things to do once the midi message has been stored. */

	std::function<void(MidiEvent)> m_learnCb;

	model::Model& m_model;
};
} // namespace giada::m

#endif
