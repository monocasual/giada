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

#include "core/channelsEngine.h"
#include "core/channels/channelManager.h"
#include "core/engine.h"
#include "core/kernelAudio.h"
#include "core/midiSynchronizer.h"
#include "core/mixer.h"

namespace giada::m
{
ChannelsEngine::ChannelsEngine(Engine& e, Mixer& m, Sequencer& s, ChannelManager& cm, Recorder& r)
: m_engine(e)
, m_mixer(m)
, m_sequencer(s)
, m_channelManager(cm)
, m_recorder(r)
{
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::press(ID channelId, int velocity)
{
	const bool  canRecordActions = m_recorder.canRecordActions();
	const bool  canQuantize      = m_sequencer.canQuantize();
	const Frame currentFrameQ    = m_sequencer.getCurrentFrameQuantized();
	m_channelManager.keyPress(channelId, velocity, canRecordActions, canQuantize, currentFrameQ);
}

void ChannelsEngine::release(ID channelId)
{
	const bool  canRecordActions = m_recorder.canRecordActions();
	const Frame currentFrameQ    = m_sequencer.getCurrentFrameQuantized();
	m_channelManager.keyRelease(channelId, canRecordActions, currentFrameQ);
}

void ChannelsEngine::kill(ID channelId)
{
	const bool  canRecordActions = m_recorder.canRecordActions();
	const Frame currentFrameQ    = m_sequencer.getCurrentFrameQuantized();
	m_channelManager.keyKill(channelId, canRecordActions, currentFrameQ);
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::setVolume(ID channelId, float v)
{
	m_channelManager.setVolume(channelId, v);
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::setPitch(ID channelId, float v)
{
	m_channelManager.setPitch(channelId, v);
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::setPan(ID channelId, float v)
{
	m_channelManager.setPan(channelId, v);
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::toggleMute(ID channelId)
{
	m_channelManager.toggleMute(channelId);
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::toggleSolo(ID channelId)
{
	m_channelManager.toggleSolo(channelId);
	m_mixer.updateSoloCount(m_channelManager.hasSolos());
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::toggleArm(ID channelId)
{
	m_channelManager.toggleArm(channelId);
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::toggleReadActions(ID channelId)
{
	m_channelManager.toggleReadActions(channelId, m_sequencer.isRunning());
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::killReadActions(ID channelId)
{
	m_channelManager.killReadActions(channelId);
}

/* -------------------------------------------------------------------------- */

void ChannelsEngine::sendMidi(ID channelId, MidiEvent e)
{
	const bool  canRecordActions = m_recorder.canRecordActions();
	const Frame currentFrameQ    = m_sequencer.getCurrentFrameQuantized();
	m_channelManager.processMidiEvent(channelId, e, canRecordActions, currentFrameQ);
}
} // namespace giada::m
