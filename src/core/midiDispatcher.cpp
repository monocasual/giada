/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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
#include "core/mixerHandler.h"
#include "core/model/model.h"
#include "core/plugins/plugin.h"
#include "core/plugins/pluginHost.h"
#include "core/recManager.h"
#include "core/types.h"
#include "glue/events.h"
#include "glue/plugin.h"
#include "utils/log.h"
#include "utils/math.h"
#include <cassert>
#include <vector>
#include "core/kernelMidi.h"
namespace giada::m::midiDispatcher
{
namespace
{
/* cb_midiLearn, cb_data_
Callback prepared by the gdMidiGrabber window and called by midiDispatcher. It 
contains things to do once the midi message has been stored. */

std::function<void()>          signalCb_ = nullptr;
std::function<void(MidiEvent)> learnCb_  = nullptr;

/* -------------------------------------------------------------------------- */

bool isMasterMidiInAllowed_(int c)
{
	int  filter  = model::get().midiIn.filter;
	bool enabled = model::get().midiIn.enabled;
	return enabled && (filter == -1 || filter == c);
}

/* -------------------------------------------------------------------------- */

bool isChannelMidiInAllowed_(ID channelId, int c)
{
	return model::get().getChannel(channelId).midiLearner.isAllowed(c);
}

/* -------------------------------------------------------------------------- */

#ifdef WITH_VST

void processPlugins_(const std::vector<Plugin*>& plugins, const MidiEvent& midiEvent)
{
	uint32_t pure = midiEvent.getRawNoVelocity();
	float    vf   = u::math::map(midiEvent.getVelocity(), G_MAX_VELOCITY, 1.0f);

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
			c::events::setPluginParameter(p->id, param.getIndex(), vf, /*gui=*/false);
			u::log::print("  >>> [pluginId=%d paramIndex=%d] (pure=0x%X, value=%d, float=%f)\n",
			    p->id, param.getIndex(), pure, midiEvent.getVelocity(), vf);
		}
	}
}

#endif

/* -------------------------------------------------------------------------- */

void processChannels_(const MidiEvent& midiEvent)
{
	uint32_t pure = midiEvent.getRawNoVelocity();

	for (const channel::Data& c : model::get().channels)
	{

		/* Do nothing on this channel if MIDI in is disabled or filtered out for
		the current MIDI channel. */
		if (!c.midiLearner.isAllowed(midiEvent.getChannel()))
			continue;

		if (pure == c.midiLearner.keyPress.getValue())
		{
			u::log::print("  >>> keyPress, ch=%d (pure=0x%X)\n", c.id, pure);
			c::events::pressChannel(c.id, midiEvent.getVelocity(), Thread::MIDI);
		}
		else if (pure == c.midiLearner.keyRelease.getValue())
		{
			u::log::print("  >>> keyRel ch=%d (pure=0x%X)\n", c.id, pure);
			c::events::releaseChannel(c.id, Thread::MIDI);
		}
		else if (pure == c.midiLearner.mute.getValue())
		{
			u::log::print("  >>> mute ch=%d (pure=0x%X)\n", c.id, pure);
			c::events::toggleMuteChannel(c.id, Thread::MIDI);
		}
		else if (pure == c.midiLearner.kill.getValue())
		{
			u::log::print("  >>> kill ch=%d (pure=0x%X)\n", c.id, pure);
			c::events::killChannel(c.id, Thread::MIDI);
		}
		else if (pure == c.midiLearner.arm.getValue())
		{
			u::log::print("  >>> arm ch=%d (pure=0x%X)\n", c.id, pure);
			c::events::toggleArmChannel(c.id, Thread::MIDI);
		}
		else if (pure == c.midiLearner.solo.getValue())
		{
			u::log::print("  >>> solo ch=%d (pure=0x%X)\n", c.id, pure);
			c::events::toggleSoloChannel(c.id, Thread::MIDI);
		}
		else if (pure == c.midiLearner.volume.getValue())
		{
			float vf = u::math::map(midiEvent.getVelocity(), G_MAX_VELOCITY, G_MAX_VOLUME);
			u::log::print("  >>> volume ch=%d (pure=0x%X, value=%d, float=%f)\n",
			    c.id, pure, midiEvent.getVelocity(), vf);
			c::events::setChannelVolume(c.id, vf, Thread::MIDI);
		}
		else if (pure == c.midiLearner.pitch.getValue())
		{
			float vf = u::math::map(midiEvent.getVelocity(), G_MAX_VELOCITY, G_MAX_PITCH);
			u::log::print("  >>> pitch ch=%d (pure=0x%X, value=%d, float=%f)\n",
			    c.id, pure, midiEvent.getVelocity(), vf);
			c::events::setChannelPitch(c.id, vf, Thread::MIDI);
		}
		else if (pure == c.midiLearner.readActions.getValue())
		{
			u::log::print("  >>> toggle read actions ch=%d (pure=0x%X)\n", c.id, pure);
			c::events::toggleReadActionsChannel(c.id, Thread::MIDI);
		}

#ifdef WITH_VST
		/* Process learned plugins parameters. */
		processPlugins_(c.plugins, midiEvent);
#endif
		/* Redirect raw MIDI message (pure + velocity) to plug-ins in armed
		channels. */

		if (c.armed){
			// u::log::print("ARMED : %X (chan %d)\n", c.id);
			// set_thruMonitor(c.id);
			c::events::sendMidiToChannel(c.id, midiEvent, Thread::MIDI);
		}
	}
}

/* -------------------------------------------------------------------------- */

void processMaster_(const MidiEvent& midiEvent)
{
	const uint32_t       pure   = midiEvent.getRawNoVelocity();
	const model::MidiIn& midiIn = model::get().midiIn;

	if (pure == midiIn.rewind)
	{
		c::events::rewindSequencer(Thread::MIDI);
		u::log::print("  >>> rewind (master) (pure=0x%X)\n", pure);
	}
	else if (pure == midiIn.startStop)
	{
		c::events::toggleSequencer(Thread::MIDI);
		u::log::print("  >>> startStop (master) (pure=0x%X)\n", pure);
	}
	else if (pure == midiIn.actionRec)
	{
		c::events::toggleActionRecording();
		u::log::print("  >>> actionRec (master) (pure=0x%X)\n", pure);
	}
	else if (pure == midiIn.inputRec)
	{
		c::events::toggleInputRecording();
		u::log::print("  >>> inputRec (master) (pure=0x%X)\n", pure);
	}
	else if (pure == midiIn.metronome)
	{
		c::events::toggleMetronome();
		u::log::print("  >>> metronome (master) (pure=0x%X)\n", pure);
	}
	else if (pure == midiIn.volumeIn)
	{
		float vf = u::math::map(midiEvent.getVelocity(), G_MAX_VELOCITY, G_MAX_VOLUME);
		c::events::setMasterInVolume(vf, Thread::MIDI);
		u::log::print("  >>> input volume (master) (pure=0x%X, value=%d, float=%f)\n",
		    pure, midiEvent.getVelocity(), vf);
	}
	else if (pure == midiIn.volumeOut)
	{
		float vf = u::math::map(midiEvent.getVelocity(), G_MAX_VELOCITY, G_MAX_VOLUME);
		c::events::setMasterOutVolume(vf, Thread::MIDI);
		u::log::print("  >>> output volume (master) (pure=0x%X, value=%d, float=%f)\n",
		    pure, midiEvent.getVelocity(), vf);
	}
	else if (pure == midiIn.beatDouble)
	{
		c::events::multiplyBeats();
		u::log::print("  >>> sequencer x2 (master) (pure=0x%X)\n", pure);
	}
	else if (pure == midiIn.beatHalf)
	{
		c::events::divideBeats();
		u::log::print("  >>> sequencer /2 (master) (pure=0x%X)\n", pure);
	}
}

/* -------------------------------------------------------------------------- */

void learnChannel_(MidiEvent e, int param, ID channelId, std::function<void()> doneCb)
{
	if (!isChannelMidiInAllowed_(channelId, e.getChannel()))
		return;

	uint32_t raw = e.getRawNoVelocity();

	channel::Data& ch = model::get().getChannel(channelId);

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

	model::swap(model::SwapType::SOFT);

	stopLearn();
	doneCb();
}

void learnMaster_(MidiEvent e, int param, std::function<void()> doneCb)
{
	if (!isMasterMidiInAllowed_(e.getChannel()))
		return;

	uint32_t raw = e.getRawNoVelocity();

	switch (param)
	{
	case G_MIDI_IN_REWIND:
		model::get().midiIn.rewind = raw;
		break;
	case G_MIDI_IN_START_STOP:
		model::get().midiIn.startStop = raw;
		break;
	case G_MIDI_IN_ACTION_REC:
		model::get().midiIn.actionRec = raw;
		break;
	case G_MIDI_IN_INPUT_REC:
		model::get().midiIn.inputRec = raw;
		break;
	case G_MIDI_IN_METRONOME:
		model::get().midiIn.metronome = raw;
		break;
	case G_MIDI_IN_VOLUME_IN:
		model::get().midiIn.volumeIn = raw;
		break;
	case G_MIDI_IN_VOLUME_OUT:
		model::get().midiIn.volumeOut = raw;
		break;
	case G_MIDI_IN_BEAT_DOUBLE:
		model::get().midiIn.beatDouble = raw;
		break;
	case G_MIDI_IN_BEAT_HALF:
		model::get().midiIn.beatHalf = raw;
		break;
	}

	model::swap(model::SwapType::SOFT);

	stopLearn();
	doneCb();
}

#ifdef WITH_VST

void learnPlugin_(MidiEvent e, std::size_t paramIndex, ID pluginId, std::function<void()> doneCb)
{
	model::DataLock lock(model::SwapType::NONE);

	Plugin* plugin = model::find<Plugin>(pluginId);

	assert(plugin != nullptr);
	assert(paramIndex < plugin->midiInParams.size());

	plugin->midiInParams[paramIndex].setValue(e.getRawNoVelocity());

	stopLearn();
	doneCb();
}

#endif

/* -------------------------------------------------------------------------- */

void triggerSignalCb_()
{
	if (signalCb_ == nullptr)
		return;
	signalCb_();
	signalCb_ = nullptr;
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

void startChannelLearn(int param, ID channelId, std::function<void()> f)
{
	learnCb_ = [=](m::MidiEvent e) { learnChannel_(e, param, channelId, f); };
}

void startMasterLearn(int param, std::function<void()> f)
{
	learnCb_ = [=](m::MidiEvent e) { learnMaster_(e, param, f); };
}

#ifdef WITH_VST

void startPluginLearn(std::size_t paramIndex, ID pluginId, std::function<void()> f)
{
	learnCb_ = [=](m::MidiEvent e) { learnPlugin_(e, paramIndex, pluginId, f); };
}

#endif

void stopLearn()
{
	learnCb_ = nullptr;
}

/* -------------------------------------------------------------------------- */

void clearMasterLearn(int param, std::function<void()> f)
{
	learnMaster_(MidiEvent(), param, f); // Empty event (0x0)
}

void clearChannelLearn(int param, ID channelId, std::function<void()> f)
{
	learnChannel_(MidiEvent(), param, channelId, f); // Empty event (0x0)
}

#ifdef WITH_VST

void clearPluginLearn(std::size_t paramIndex, ID pluginId, std::function<void()> f)
{
	learnPlugin_(MidiEvent(), paramIndex, pluginId, f); // Empty event (0x0)
}

#endif

/* -------------------------------------------------------------------------- */
int thruMonitorCh = 144;

void set_thruMonitor(int thruCh){
	thruMonitorCh = thruCh;
	u::log::print("thruMonitor Ch %X \n", thruCh);
}

void dispatch(int byte1, int byte2, int byte3)
{
	/* Here we want to catch two things: a) note on/note off from a MIDI keyboard 
	and b) knob/wheel/slider movements from a MIDI controller. 
	We must also fix the velocity zero issue for those devices that sends NOTE
	OFF events as NOTE ON + velocity zero. Let's make it a real NOTE OFF event. */

	MidiEvent midiEvent(byte1, byte2, byte3);
	midiEvent.fixVelocityZero();

	u::log::print("[midiDispatcher] MIDI received - 0x%X (chan %d)\n", midiEvent.getRaw(),
	    midiEvent.getChannel());

	// can I get the armed ch/ch's to put instead of 144 ? 
	kernelMidi::send_thru(thruMonitorCh,byte2,byte3);
	/* Start dispatcher. Don't parse channels if MIDI learn is ON, just learn 
	the incoming MIDI signal. The action is not invoked directly, but scheduled 
	to be perfomed by the Event Dispatcher. */

	Action                     action = {0, 0, 0, midiEvent};
	eventDispatcher::EventType event  = learnCb_ != nullptr ? eventDispatcher::EventType::MIDI_DISPATCHER_LEARN : eventDispatcher::EventType::MIDI_DISPATCHER_PROCESS;

	eventDispatcher::pumpMidiEvent({event, 0, 0, action});
}

/* -------------------------------------------------------------------------- */

void learn(const MidiEvent& e)
{
	assert(learnCb_ != nullptr);
	learnCb_(e);
}

/* -------------------------------------------------------------------------- */

void process(const MidiEvent& e)
{
	processMaster_(e);
	processChannels_(e);
	triggerSignalCb_();
}

/* -------------------------------------------------------------------------- */

void setSignalCallback(std::function<void()> f)
{
	signalCb_ = f;
}
} // namespace giada::m::midiDispatcher

