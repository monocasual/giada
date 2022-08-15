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

#include "core/midiDispatcher.h"
#include "core/conf.h"
#include "core/mixer.h"
#include "core/model/model.h"
#include "core/plugins/plugin.h"
#include "core/plugins/pluginHost.h"
#include "core/recorder.h"
#include "core/types.h"
#include "glue/events.h"
#include "glue/plugin.h"
#include "utils/log.h"
#include "utils/math.h"
#include <cassert>
#include <cstddef>
#include <vector>

namespace giada::m
{
MidiDispatcher::MidiDispatcher(model::Model& m)
: onDispatch(nullptr)
, m_learnCb(nullptr)
, m_model(m)
{
}

/* -------------------------------------------------------------------------- */

void MidiDispatcher::startChannelLearn(int param, ID channelId, std::function<void()> f)
{
	m_learnCb = [=](m::MidiEvent e) { learnChannel(e, param, channelId, f); };
}

void MidiDispatcher::startMasterLearn(int param, std::function<void()> f)
{
	m_learnCb = [=](m::MidiEvent e) { learnMaster(e, param, f); };
}

void MidiDispatcher::startPluginLearn(std::size_t paramIndex, ID pluginId, std::function<void()> f)
{
	m_learnCb = [=](m::MidiEvent e) { learnPlugin(e, paramIndex, pluginId, f); };
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
	assert(onDispatch != nullptr);

	/* Here we want to catch two things: a) note on/note off from a MIDI keyboard 
	and b) knob/wheel/slider movements from a MIDI controller. 
	We must also fix the velocity zero issue for those devices that sends NOTE
	OFF events as NOTE ON + velocity zero. Let's make it a real NOTE OFF event. */

	MidiEvent eFixed = e;
	eFixed.fixVelocityZero();

	Action action    = {0, 0, 0, eFixed};
	auto   eventType = m_learnCb != nullptr ? EventDispatcher::EventType::MIDI_DISPATCHER_LEARN : EventDispatcher::EventType::MIDI_DISPATCHER_PROCESS;

	onDispatch(eventType, action);
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

	processMaster(e);
	processChannels(e);
	onEventReceived();
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
	return m_model.get().getChannel(channelId).midiLearner.isAllowed(c);
}

/* -------------------------------------------------------------------------- */

void MidiDispatcher::processPlugins(ID channelId, const std::vector<Plugin*>& plugins,
    const MidiEvent& midiEvent)
{
	const uint32_t pure = midiEvent.getRawNoVelocity();
	const float    vf   = u::math::map(midiEvent.getVelocity(), G_MAX_VELOCITY, 1.0f);

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
			c::events::setPluginParameter(channelId, p->id, param.getIndex(), vf, Thread::MIDI);
			G_DEBUG("   [pluginId={} paramIndex={}] (pure=0x{:0X}, value={}, float={})",
			    p->id, param.getIndex(), pure, midiEvent.getVelocity(), vf);
		}
	}
}

/* -------------------------------------------------------------------------- */

void MidiDispatcher::processChannels(const MidiEvent& midiEvent)
{
	const uint32_t pure = midiEvent.getRawNoVelocity();

	for (const Channel& c : m_model.get().channels)
	{
		/* Do nothing on this channel if MIDI in is disabled or filtered out for
		the current MIDI channel. */

		if (!c.midiLearner.isAllowed(midiEvent.getChannel()))
			continue;

		if (pure == c.midiLearner.keyPress.getValue())
		{
			G_DEBUG("   keyPress, ch={} (pure=0x{:0X})", c.id, pure);
			c::events::pressChannel(c.id, midiEvent.getVelocity(), Thread::MIDI);
		}
		else if (pure == c.midiLearner.keyRelease.getValue())
		{
			G_DEBUG("   keyRel ch={} (pure=0x{:0X})", c.id, pure);
			c::events::releaseChannel(c.id, Thread::MIDI);
		}
		else if (pure == c.midiLearner.mute.getValue())
		{
			G_DEBUG("   mute ch={} (pure=0x{:0X})", c.id, pure);
			c::events::toggleMuteChannel(c.id, Thread::MIDI);
		}
		else if (pure == c.midiLearner.kill.getValue())
		{
			G_DEBUG("   kill ch={} (pure=0x{:0X})", c.id, pure);
			c::events::killChannel(c.id, Thread::MIDI);
		}
		else if (pure == c.midiLearner.arm.getValue())
		{
			G_DEBUG("   arm ch={} (pure=0x{:0X})", c.id, pure);
			c::events::toggleArmChannel(c.id, Thread::MIDI);
		}
		else if (pure == c.midiLearner.solo.getValue())
		{
			G_DEBUG("   solo ch={} (pure=0x{:0X})", c.id, pure);
			c::events::toggleSoloChannel(c.id, Thread::MIDI);
		}
		else if (pure == c.midiLearner.volume.getValue())
		{
			float vf = u::math::map(midiEvent.getVelocity(), G_MAX_VELOCITY, G_MAX_VOLUME);
			G_DEBUG("   volume ch={} (pure=0x{:0X}, value=%d, float=%f)",
			    c.id, pure, midiEvent.getVelocity(), vf);
			c::events::setChannelVolume(c.id, vf, Thread::MIDI);
		}
		else if (pure == c.midiLearner.pitch.getValue())
		{
			float vf = u::math::map(midiEvent.getVelocity(), G_MAX_VELOCITY, G_MAX_PITCH);
			G_DEBUG("   pitch ch={} (pure=0x{:0X}, value=%d, float=%f)",
			    c.id, pure, midiEvent.getVelocity(), vf);
			c::events::setChannelPitch(c.id, vf, Thread::MIDI);
		}
		else if (pure == c.midiLearner.readActions.getValue())
		{
			G_DEBUG("   toggle read actions ch={} (pure=0x{:0X})", c.id, pure);
			c::events::toggleReadActionsChannel(c.id, Thread::MIDI);
		}

		/* Process learned plugins parameters. */
		processPlugins(c.id, c.plugins, midiEvent);

		/* Redirect raw MIDI message (pure + velocity) to plug-ins in armed
		channels. */
		if (c.armed)
			c::events::sendMidiToChannel(c.id, midiEvent, Thread::MIDI);
	}
}

/* -------------------------------------------------------------------------- */

void MidiDispatcher::processMaster(const MidiEvent& midiEvent)
{
	const uint32_t       pure   = midiEvent.getRawNoVelocity();
	const model::MidiIn& midiIn = m_model.get().midiIn;

	if (pure == midiIn.rewind)
	{
		c::events::rewindSequencer(Thread::MIDI);
		G_DEBUG("   rewind (master) (pure=0x{:0X})", pure);
	}
	else if (pure == midiIn.startStop)
	{
		c::events::toggleSequencer(Thread::MIDI);
		G_DEBUG("   startStop (master) (pure=0x{:0X})", pure);
	}
	else if (pure == midiIn.actionRec)
	{
		c::events::toggleActionRecording();
		G_DEBUG("   actionRec (master) (pure=0x{:0X})", pure);
	}
	else if (pure == midiIn.inputRec)
	{
		c::events::toggleInputRecording();
		G_DEBUG("   inputRec (master) (pure=0x{:0X})", pure);
	}
	else if (pure == midiIn.metronome)
	{
		c::events::toggleMetronome();
		G_DEBUG("   metronome (master) (pure=0x{:0X})", pure);
	}
	else if (pure == midiIn.volumeIn)
	{
		float vf = u::math::map(midiEvent.getVelocity(), G_MAX_VELOCITY, G_MAX_VOLUME);
		c::events::setMasterInVolume(vf, Thread::MIDI);
		G_DEBUG("   input volume (master) (pure=0x{:0X}, value={}, float={})",
		    pure, midiEvent.getVelocity(), vf);
	}
	else if (pure == midiIn.volumeOut)
	{
		float vf = u::math::map(midiEvent.getVelocity(), G_MAX_VELOCITY, G_MAX_VOLUME);
		c::events::setMasterOutVolume(vf, Thread::MIDI);
		G_DEBUG("   output volume (master) (pure=0x{:0X}, value={}, float={})",
		    pure, midiEvent.getVelocity(), vf);
	}
	else if (pure == midiIn.beatDouble)
	{
		c::events::multiplyBeats();
		G_DEBUG("   sequencer x2 (master) (pure=0x{:0X})", pure);
	}
	else if (pure == midiIn.beatHalf)
	{
		c::events::divideBeats();
		G_DEBUG("   sequencer /2 (master) (pure=0x{:0X})", pure);
	}
}

/* -------------------------------------------------------------------------- */

void MidiDispatcher::learnChannel(MidiEvent e, int param, ID channelId, std::function<void()> doneCb)
{
	if (!isChannelMidiInAllowed(channelId, e.getChannel()))
		return;

	const uint32_t raw = e.getRawNoVelocity();

	Channel& ch = m_model.get().getChannel(channelId);

	switch (param)
	{
	case G_MIDI_IN_KEYPRESS:
		ch.midiLearner.keyPress.setValue(raw);
		break;
	case G_MIDI_IN_KEYREL:
		ch.midiLearner.keyRelease.setValue(raw);
		break;
	case G_MIDI_IN_KILL:
		ch.midiLearner.kill.setValue(raw);
		break;
	case G_MIDI_IN_ARM:
		ch.midiLearner.arm.setValue(raw);
		break;
	case G_MIDI_IN_MUTE:
		ch.midiLearner.mute.setValue(raw);
		break;
	case G_MIDI_IN_SOLO:
		ch.midiLearner.solo.setValue(raw);
		break;
	case G_MIDI_IN_VOLUME:
		ch.midiLearner.volume.setValue(raw);
		break;
	case G_MIDI_IN_PITCH:
		ch.midiLearner.pitch.setValue(raw);
		break;
	case G_MIDI_IN_READ_ACTIONS:
		ch.midiLearner.readActions.setValue(raw);
		break;
	case G_MIDI_OUT_L_PLAYING:
		ch.midiLighter.playing.setValue(raw);
		break;
	case G_MIDI_OUT_L_MUTE:
		ch.midiLighter.mute.setValue(raw);
		break;
	case G_MIDI_OUT_L_SOLO:
		ch.midiLighter.solo.setValue(raw);
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
	model::DataLock lock   = m_model.lockData(model::SwapType::NONE);
	Plugin*         plugin = m_model.findShared<Plugin>(pluginId);

	assert(plugin != nullptr);
	assert(paramIndex < plugin->midiInParams.size());

	plugin->midiInParams[paramIndex].setValue(e.getRawNoVelocity());

	stopLearn();
	doneCb();
}
} // namespace giada::m
