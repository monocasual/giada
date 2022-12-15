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

#ifndef G_CHANNELS_ENGINE_H
#define G_CHANNELS_ENGINE_H

#include "core/types.h"

namespace giada::m
{
class MidiEvent;
class Engine;
class KernelAudio;
class Mixer;
class Sequencer;
class MidiSynchronizer;
class ChannelManager;
class Recorder;
class ChannelsEngine
{
public:
	ChannelsEngine(Engine&, KernelAudio&, Mixer&, Sequencer&, MidiSynchronizer&, ChannelManager&, Recorder&);

	void press(ID, int velocity);
	void release(ID);
	void kill(ID);
	void setVolume(ID, float);
	void setPitch(ID, float);
	void setPan(ID, float);
	void toggleMute(ID);
	void toggleSolo(ID);
	void toggleArm(ID);
	void toggleReadActions(ID);
	void killReadActions(ID);
	void sendMidi(ID, MidiEvent);

private:
	Engine&           m_engine;
	KernelAudio&      m_kernelAudio;
	Mixer&            m_mixer;
	Sequencer&        m_sequencer;
	MidiSynchronizer& m_midiSynchronizer;
	ChannelManager&   m_channelManager;
	Recorder&         m_recorder;
};
} // namespace giada::m

#endif
