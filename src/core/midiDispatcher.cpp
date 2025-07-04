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

#include "src/core/midiDispatcher.h"
#include "src/core/conf.h"
#include "src/core/mixer.h"
#include "src/core/model/model.h"
#include "src/core/plugins/plugin.h"
#include "src/core/plugins/pluginHost.h"
#include "src/core/recorder.h"
#include "src/core/types.h"
#include "src/glue/channel.h"
#include "src/glue/main.h"
#include "src/glue/plugin.h"
#include "src/utils/log.h"
#include "src/utils/math.h"
#include <cassert>
#include <cstddef>
#include <vector>

namespace giada::m
{
MidiDispatcher::MidiDispatcher(model::Model& m)
: m_learnCb(nullptr)
, m_model(m)
{
}

/* -------------------------------------------------------------------------- */

void MidiDispatcher::startChannelLearn(int param, ID channelId, std::function<void()> f)
{
	m_learnCb = [this, param, channelId, f](MidiEvent e)
	{ learnChannel(e, param, channelId, f); };
}

void MidiDispatcher::startMasterLearn(int param, std::function<void()> f)
{
	m_learnCb = [this, param, f](MidiEvent e)
	{ learnMaster(e, param, f); };
}

void MidiDispatcher::startPluginLearn(std::size_t paramIndex, ID pluginId, std::function<void()> f)
{
	m_learnCb = [this, paramIndex, pluginId, f](MidiEvent e)
	{ learnPlugin(e, paramIndex, pluginId, f); };
}

void MidiDispatcher::stopLearn()
{
	m_learnCb = nullptr;
}

/* -------------------------------------------------------------------------- */

void MidiDispatcher::clearMasterLearn(int param, std::function<void()> f)
{
	learnMaster(MidiEvent(), param, f); // Empty event (0x0)
}

void MidiDispatcher::clearChannelLearn(int param, ID channelId, std::function<void()> f)
{
	learnChannel(MidiEvent(), param, channelId, f); // Empty event (0x0)
}

void MidiDispatcher::clearPluginLearn(std::size_t paramIndex, ID pluginId, std::function<void()> f)
{
	learnPlugin(MidiEvent(), paramIndex, pluginId, f); // Empty event (0x0)
}

/* -------------------------------------------------------------------------- */

void MidiDispatcher::dispatch(const MidiEvent& e)
{
	/* Fix the velocity zero issue for those devices that sends NOTE OFF events
	as NOTE ON + velocity zero. Let's make it a real NOTE OFF event. */

	MidiEvent eFixed = e;
	eFixed.fixVelocityZero();

	/* If learn callback is set, a MIDI learn session is in progress. Otherwise
	is just normal dispatching. */

	if (m_learnCb != nullptr)
		learn(eFixed);
	else
		process(eFixed);
}

/* -------------------------------------------------------------------------- */

void MidiDispatcher::learn(const MidiEvent& e)
{
	assert(m_learnCb != nullptr);
	m_learnCb(e);
}

/* -------------------------------------------------------------------------- */

void MidiDispatcher::process(const MidiEvent& e)
{
	assert(onEventReceived != nullptr);
	assert(e.getType() != MidiEvent::Type::INVALID);

	/* Here we are interested only in CHANNEL events, that is note on/note off
	from a MIDI keyboard, knob/wheel/slider movements from a MIDI controller,
	and so on. SYSTEM events (MIDI Clock, ...) are ignored. */

	if (e.getType() != MidiEvent::Type::CHANNEL)
		return;

	onEventReceived();
	processMaster(e);
	processTracks(e);
}

/* -------------------------------------------------------------------------- */

bool MidiDispatcher::isMasterMidiInAllowed(int c)
{
	int  filter  = m_model.get().midiIn.filter;
	bool enabled = m_model.get().midiIn.enabled;
	return enabled && (filter == -1 || filter == c);
}

/* -------------------------------------------------------------------------- */

bool MidiDispatcher::isChannelMidiInAllowed(ID channelId, int c)
{
	return m_model.get().tracks.getChannel(channelId).midiInput.isAllowed(c);
}

/* -------------------------------------------------------------------------- */

void MidiDispatcher::processPlugins(ID channelId, const std::vector<Plugin*>& plugins,
    const MidiEvent& midiEvent)
{
	const uint32_t pure      = midiEvent.getRawNoVelocity();
	const float    velocityF = midiEvent.getVelocityFloat();

	/* Plugins' parameters layout reflects the structure of the matrix
	Channel::midiInPlugins. It is safe to assume then that Plugin 'p' and
	parameter indexes match both the structure of Channel::midiInPlugins and the
	vector of plugins. */

	for (Plugin* p : plugins)
	{
		for (const MidiLearnParam& param : p->midiInParams)
		{
			if (pure != param.getValue())
				continue;
			c::plugin::setParameter(channelId, p->id, param.getIndex(), velocityF, Thread::MIDI);
			G_DEBUG("   [pluginId={} paramIndex={}] (pure=0x{:0X}, value={}, float={})",
			    p->id, param.getIndex(), pure, midiEvent.getVelocity(), velocityF);
		}
	}
}

/* -------------------------------------------------------------------------- */

void MidiDispatcher::processTracks(const MidiEvent& midiEvent)
{
	for (const model::Track& track : m_model.get().tracks.getAll())
		processChannels(track.getChannels().getAll(), midiEvent);
}

/* -------------------------------------------------------------------------- */

void MidiDispatcher::processChannels(const std::vector<Channel>& channels, const MidiEvent& midiEvent)
{
	for (const Channel& ch : channels)
		processChannel(ch, midiEvent);
}

/* -------------------------------------------------------------------------- */

void MidiDispatcher::processChannel(const Channel& c, const MidiEvent& midiEvent)
{
	/* Do nothing on this channel if MIDI in is disabled or filtered out for
	the current MIDI channel. */

	if (!c.midiInput.isAllowed(midiEvent.getChannel()))
		return;

	const uint32_t pure = midiEvent.getRawNoVelocity();

	if (pure == c.midiInput.keyPress.getValue())
	{
		G_DEBUG("   keyPress, ch={} (pure=0x{:0X})", c.id, pure);
		c::channel::pressChannel(c.id, midiEvent.getVelocityFloat(), Thread::MIDI);
	}
	else if (pure == c.midiInput.keyRelease.getValue())
	{
		G_DEBUG("   keyRel ch={} (pure=0x{:0X})", c.id, pure);
		c::channel::releaseChannel(c.id, Thread::MIDI);
	}
	else if (pure == c.midiInput.mute.getValue())
	{
		G_DEBUG("   mute ch={} (pure=0x{:0X})", c.id, pure);
		c::channel::toggleMuteChannel(c.id, Thread::MIDI);
	}
	else if (pure == c.midiInput.kill.getValue())
	{
		G_DEBUG("   kill ch={} (pure=0x{:0X})", c.id, pure);
		c::channel::killChannel(c.id, Thread::MIDI);
	}
	else if (pure == c.midiInput.arm.getValue())
	{
		G_DEBUG("   arm ch={} (pure=0x{:0X})", c.id, pure);
		c::channel::toggleArmChannel(c.id, Thread::MIDI);
	}
	else if (pure == c.midiInput.solo.getValue())
	{
		G_DEBUG("   solo ch={} (pure=0x{:0X})", c.id, pure);
		c::channel::toggleSoloChannel(c.id, Thread::MIDI);
	}
	else if (pure == c.midiInput.volume.getValue())
	{
		G_DEBUG("   volume ch={} (pure=0x{:0X}, value={})", c.id, pure, midiEvent.getVelocityFloat());
		c::channel::setChannelVolume(c.id, midiEvent.getVelocityFloat(), Thread::MIDI);
	}
	else if (pure == c.midiInput.pitch.getValue())
	{
		G_DEBUG("   pitch ch={} (pure=0x{:0X}, value={})", c.id, pure, midiEvent.getVelocityFloat());
		c::channel::setChannelPitch(c.id, midiEvent.getVelocityFloat(), Thread::MIDI);
	}
	else if (pure == c.midiInput.readActions.getValue())
	{
		G_DEBUG("   toggle read actions ch={} (pure=0x{:0X})", c.id, pure);
		c::channel::toggleReadActionsChannel(c.id, Thread::MIDI);
	}

	/* Process learned plugins parameters. */
	processPlugins(c.id, c.plugins, midiEvent);

	/* Redirect raw MIDI message (pure + velocity) to plug-ins in armed MIDI channels. */
	if (c.armed && c.type == ChannelType::MIDI)
		c::channel::sendMidiToChannel(c.id, midiEvent, Thread::MIDI);
}

/* -------------------------------------------------------------------------- */

void MidiDispatcher::processMaster(const MidiEvent& midiEvent)
{
	const uint32_t       pure   = midiEvent.getRawNoVelocity();
	const model::MidiIn& midiIn = m_model.get().midiIn;

	if (pure == midiIn.rewind)
	{
		c::main::rewindSequencer();
		G_DEBUG("   rewind (master) (pure=0x{:0X})", pure);
	}
	else if (pure == midiIn.startStop)
	{
		c::main::toggleSequencer();
		G_DEBUG("   startStop (master) (pure=0x{:0X})", pure);
	}
	else if (pure == midiIn.actionRec)
	{
		c::main::toggleActionRecording();
		G_DEBUG("   actionRec (master) (pure=0x{:0X})", pure);
	}
	else if (pure == midiIn.inputRec)
	{
		c::main::toggleInputRecording();
		G_DEBUG("   inputRec (master) (pure=0x{:0X})", pure);
	}
	else if (pure == midiIn.metronome)
	{
		c::main::toggleMetronome();
		G_DEBUG("   metronome (master) (pure=0x{:0X})", pure);
	}
	else if (pure == midiIn.volumeIn)
	{
		c::main::setMasterInVolume(midiEvent.getVelocityFloat(), Thread::MIDI);
		G_DEBUG("   input volume (master) (pure=0x{:0X}, value={})", pure, midiEvent.getVelocityFloat());
	}
	else if (pure == midiIn.volumeOut)
	{
		c::main::setMasterOutVolume(midiEvent.getVelocityFloat(), Thread::MIDI);
		G_DEBUG("   output volume (master) (pure=0x{:0X}, value={})", pure, midiEvent.getVelocityFloat());
	}
	else if (pure == midiIn.beatDouble)
	{
		c::main::multiplyBeats();
		G_DEBUG("   sequencer x2 (master) (pure=0x{:0X})", pure);
	}
	else if (pure == midiIn.beatHalf)
	{
		c::main::divideBeats();
		G_DEBUG("   sequencer /2 (master) (pure=0x{:0X})", pure);
	}
}

/* -------------------------------------------------------------------------- */

void MidiDispatcher::learnChannel(MidiEvent e, int param, ID channelId, std::function<void()> doneCb)
{
	if (!isChannelMidiInAllowed(channelId, e.getChannel()))
		return;

	const uint32_t raw = e.getRawNoVelocity();

	Channel& ch = m_model.get().tracks.getChannel(channelId);

	switch (param)
	{
	case G_MIDI_IN_KEYPRESS:
		ch.midiInput.keyPress.setValue(raw);
		break;
	case G_MIDI_IN_KEYREL:
		ch.midiInput.keyRelease.setValue(raw);
		break;
	case G_MIDI_IN_KILL:
		ch.midiInput.kill.setValue(raw);
		break;
	case G_MIDI_IN_ARM:
		ch.midiInput.arm.setValue(raw);
		break;
	case G_MIDI_IN_MUTE:
		ch.midiInput.mute.setValue(raw);
		break;
	case G_MIDI_IN_SOLO:
		ch.midiInput.solo.setValue(raw);
		break;
	case G_MIDI_IN_VOLUME:
		ch.midiInput.volume.setValue(raw);
		break;
	case G_MIDI_IN_PITCH:
		ch.midiInput.pitch.setValue(raw);
		break;
	case G_MIDI_IN_READ_ACTIONS:
		ch.midiInput.readActions.setValue(raw);
		break;
	case G_MIDI_OUT_L_PLAYING:
		ch.midiLightning.playing.setValue(raw);
		break;
	case G_MIDI_OUT_L_MUTE:
		ch.midiLightning.mute.setValue(raw);
		break;
	case G_MIDI_OUT_L_SOLO:
		ch.midiLightning.solo.setValue(raw);
		break;
	}

	m_model.swap(model::SwapType::SOFT);

	stopLearn();
	doneCb();
}

/* -------------------------------------------------------------------------- */

void MidiDispatcher::learnMaster(MidiEvent e, int param, std::function<void()> doneCb)
{
	if (!isMasterMidiInAllowed(e.getChannel()))
		return;

	const uint32_t raw = e.getRawNoVelocity();

	switch (param)
	{
	case G_MIDI_IN_REWIND:
		m_model.get().midiIn.rewind = raw;
		break;
	case G_MIDI_IN_START_STOP:
		m_model.get().midiIn.startStop = raw;
		break;
	case G_MIDI_IN_ACTION_REC:
		m_model.get().midiIn.actionRec = raw;
		break;
	case G_MIDI_IN_INPUT_REC:
		m_model.get().midiIn.inputRec = raw;
		break;
	case G_MIDI_IN_METRONOME:
		m_model.get().midiIn.metronome = raw;
		break;
	case G_MIDI_IN_VOLUME_IN:
		m_model.get().midiIn.volumeIn = raw;
		break;
	case G_MIDI_IN_VOLUME_OUT:
		m_model.get().midiIn.volumeOut = raw;
		break;
	case G_MIDI_IN_BEAT_DOUBLE:
		m_model.get().midiIn.beatDouble = raw;
		break;
	case G_MIDI_IN_BEAT_HALF:
		m_model.get().midiIn.beatHalf = raw;
		break;
	}

	m_model.swap(model::SwapType::SOFT);

	stopLearn();
	doneCb();
}

/* -------------------------------------------------------------------------- */

void MidiDispatcher::learnPlugin(MidiEvent e, std::size_t paramIndex, ID pluginId, std::function<void()> doneCb)
{
	model::SharedLock lock   = m_model.lockShared(model::SwapType::NONE);
	Plugin*           plugin = m_model.findPlugin(pluginId);

	assert(plugin != nullptr);
	assert(paramIndex < plugin->midiInParams.size());

	plugin->midiInParams[paramIndex].setValue(e.getRawNoVelocity());

	stopLearn();
	doneCb();
}
} // namespace giada::m
