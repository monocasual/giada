/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#include <cassert>
#include <vector>
#include "glue/plugin.h"
#include "glue/events.h"
#include "utils/log.h"
#include "utils/math.h"
#include "core/model/model.h"
#include "core/conf.h"
#include "core/mixer.h"
#include "core/mixerHandler.h"
#include "core/plugins/pluginHost.h"
#include "core/plugins/plugin.h"
#include "core/recManager.h"
#include "core/types.h"
#include "core/midiDispatcher.h"


namespace giada {
namespace m {
namespace midiDispatcher
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
	model::MidiInLock l(model::midiIn);
	int filter   = model::midiIn.get()->filter;
	bool enabled = model::midiIn.get()->enabled;
	return enabled && (filter == -1 || filter == c);
}


/* -------------------------------------------------------------------------- */


bool isChannelMidiInAllowed_(ID channelId, int c)
{
	model::ChannelsLock l(model::channels);
	return model::get(model::channels, channelId).midiLearner.state->isAllowed(c);
}


/* -------------------------------------------------------------------------- */


#ifdef WITH_VST

void processPlugins_(const std::vector<ID>& ids, const MidiEvent& midiEvent)
{
	uint32_t pure = midiEvent.getRawNoVelocity();
	float    vf   = u::math::map(midiEvent.getVelocity(), G_MAX_VELOCITY, 1.0f);

	/* Plugins' parameters layout reflects the structure of the matrix
	Channel::midiInPlugins. It is safe to assume then that Plugin 'p' and 
	parameter indexes match both the structure of Channel::midiInPlugins and the 
	vector of plugins. */

	m::model::PluginsLock l(m::model::plugins);
	for (ID id : ids) {
		const m::Plugin& p = m::model::get(m::model::plugins, id);
		for (const MidiLearnParam& param : p.midiInParams) {
			if (pure != param.getValue())
				continue;
			c::events::setPluginParameter(id, param.getIndex(), vf, /*gui=*/false);
			u::log::print("  >>> [pluginId=%d paramIndex=%d] (pure=0x%X, value=%d, float=%f)\n",
				p.id, param.getIndex(), pure, midiEvent.getVelocity(), vf);
		}
	}
}

#endif


/* -------------------------------------------------------------------------- */


void processChannels_(const MidiEvent& midiEvent)
{
	uint32_t pure = midiEvent.getRawNoVelocity();

	model::ChannelsLock lock(model::channels);

	for (const Channel* c : model::channels) {

		/* Do nothing on this channel if MIDI in is disabled or filtered out for
		the current MIDI channel. */

		if (!c->midiLearner.state->isAllowed(midiEvent.getChannel()))
			continue;

		if (pure == c->midiLearner.state->keyPress.getValue()) {
			u::log::print("  >>> keyPress, ch=%d (pure=0x%X)\n", c->id, pure);
			c::events::pressChannel(c->id, midiEvent.getVelocity(), Thread::MIDI);
		}
		else if (pure == c->midiLearner.state->keyRelease.getValue()) {
			u::log::print("  >>> keyRel ch=%d (pure=0x%X)\n", c->id, pure);
			c::events::releaseChannel(c->id, Thread::MIDI);
		}
		else if (pure == c->midiLearner.state->mute.getValue()) {
			u::log::print("  >>> mute ch=%d (pure=0x%X)\n", c->id, pure);
			c::events::toggleMuteChannel(c->id, Thread::MIDI);
		}		
		else if (pure == c->midiLearner.state->kill.getValue()) {
			u::log::print("  >>> kill ch=%d (pure=0x%X)\n", c->id, pure);
			c::events::killChannel(c->id, Thread::MIDI);
		}		
		else if (pure == c->midiLearner.state->arm.getValue()) {
			u::log::print("  >>> arm ch=%d (pure=0x%X)\n", c->id, pure);
			c::events::toggleArmChannel(c->id, Thread::MIDI);
		}
		else if (pure == c->midiLearner.state->solo.getValue()) {
			u::log::print("  >>> solo ch=%d (pure=0x%X)\n", c->id, pure);
			c::events::toggleSoloChannel(c->id, Thread::MIDI);
		}
		else if (pure == c->midiLearner.state->volume.getValue()) {
			float vf = u::math::map(midiEvent.getVelocity(), G_MAX_VELOCITY, G_MAX_VOLUME); 
			u::log::print("  >>> volume ch=%d (pure=0x%X, value=%d, float=%f)\n", 
				c->id, pure, midiEvent.getVelocity(), vf);
			c::events::setChannelVolume(c->id, vf, Thread::MIDI);
		}
		else if (pure == c->midiLearner.state->pitch.getValue()) {
			float vf = u::math::map(midiEvent.getVelocity(), G_MAX_VELOCITY, G_MAX_PITCH); 
			u::log::print("  >>> pitch ch=%d (pure=0x%X, value=%d, float=%f)\n",
				c->id, pure, midiEvent.getVelocity(), vf);
			c::events::setChannelPitch(c->id, vf, Thread::MIDI);
		}
		else if (pure == c->midiLearner.state->readActions.getValue()) {
			u::log::print("  >>> toggle read actions ch=%d (pure=0x%X)\n", c->id, pure);
			c::events::toggleReadActionsChannel(c->id, Thread::MIDI);
		}

#ifdef WITH_VST
		/* Process learned plugins parameters. */
		processPlugins_(c->pluginIds, midiEvent); 
#endif

		/* Redirect raw MIDI message (pure + velocity) to plug-ins in armed
		channels. */

		if (c->state->armed.load() == true)
			c::events::sendMidiToChannel(c->id, midiEvent, Thread::MIDI);
	}
}


/* -------------------------------------------------------------------------- */


void processMaster_(const MidiEvent& midiEvent)
{
	m::model::MidiInLock l(m::model::midiIn);

	const uint32_t       pure   = midiEvent.getRawNoVelocity();
	const model::MidiIn* midiIn = model::midiIn.get();

	if      (pure == midiIn->rewind) {
		c::events::rewindSequencer(Thread::MIDI);
		u::log::print("  >>> rewind (master) (pure=0x%X)\n", pure);
	}
	else if (pure == midiIn->startStop) {
		c::events::toggleSequencer(Thread::MIDI);
		u::log::print("  >>> startStop (master) (pure=0x%X)\n", pure);
	}
	else if (pure == midiIn->actionRec) {
		c::events::toggleActionRecording();
		u::log::print("  >>> actionRec (master) (pure=0x%X)\n", pure);
	}
	else if (pure == midiIn->inputRec) {
		c::events::toggleInputRecording();
		u::log::print("  >>> inputRec (master) (pure=0x%X)\n", pure);
	}
	else if (pure == midiIn->metronome) {
		c::events::toggleMetronome();
		u::log::print("  >>> metronome (master) (pure=0x%X)\n", pure);
	}
	else if (pure == midiIn->volumeIn) {
		float vf = u::math::map(midiEvent.getVelocity(), G_MAX_VELOCITY, G_MAX_VOLUME); 
		c::events::setMasterInVolume(vf, Thread::MIDI);
		u::log::print("  >>> input volume (master) (pure=0x%X, value=%d, float=%f)\n",
			pure, midiEvent.getVelocity(), vf);
	}
	else if (pure == midiIn->volumeOut) {
		float vf = u::math::map(midiEvent.getVelocity(), G_MAX_VELOCITY, G_MAX_VOLUME); 
		c::events::setMasterOutVolume(vf, Thread::MIDI);
		u::log::print("  >>> output volume (master) (pure=0x%X, value=%d, float=%f)\n",
			pure, midiEvent.getVelocity(), vf);
	}
	else if (pure == midiIn->beatDouble) {
		c::events::multiplyBeats();
		u::log::print("  >>> sequencer x2 (master) (pure=0x%X)\n", pure);
	}
	else if (pure == midiIn->beatHalf) {
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

	model::onGet(model::channels, channelId, [param, raw](Channel& c)
	{	
		switch (param) {
			case G_MIDI_IN_KEYPRESS:     c.midiLearner.state->keyPress.setValue(raw);    break;
			case G_MIDI_IN_KEYREL:       c.midiLearner.state->keyRelease.setValue(raw);  break;
			case G_MIDI_IN_KILL:         c.midiLearner.state->kill.setValue(raw);        break;
			case G_MIDI_IN_ARM:          c.midiLearner.state->arm.setValue(raw);         break;
			case G_MIDI_IN_MUTE:         c.midiLearner.state->mute.setValue(raw);        break;
			case G_MIDI_IN_SOLO:         c.midiLearner.state->solo.setValue(raw);        break;
			case G_MIDI_IN_VOLUME:       c.midiLearner.state->volume.setValue(raw);      break;
			case G_MIDI_IN_PITCH:        c.midiLearner.state->pitch.setValue(raw);       break;
			case G_MIDI_IN_READ_ACTIONS: c.midiLearner.state->readActions.setValue(raw); break;
			case G_MIDI_OUT_L_PLAYING:   c.midiLighter.state->playing.setValue(raw);     break;
			case G_MIDI_OUT_L_MUTE:      c.midiLighter.state->mute.setValue(raw);        break;
			case G_MIDI_OUT_L_SOLO:      c.midiLighter.state->solo.setValue(raw);        break;
		}
	});

	stopLearn();
	doneCb();
}


void learnMaster_(MidiEvent e, int param, std::function<void()> doneCb)
{
	if (!isMasterMidiInAllowed_(e.getChannel()))
		return;

	uint32_t raw = e.getRawNoVelocity();

	model::onSwap(model::midiIn, [param, raw](model::MidiIn& m)
	{
		switch (param) {
			case G_MIDI_IN_REWIND:      m.rewind     = raw; break;
			case G_MIDI_IN_START_STOP:  m.startStop  = raw; break;
			case G_MIDI_IN_ACTION_REC:  m.actionRec  = raw; break;
			case G_MIDI_IN_INPUT_REC:   m.inputRec   = raw; break;
			case G_MIDI_IN_METRONOME:   m.metronome  = raw; break;
			case G_MIDI_IN_VOLUME_IN:   m.volumeIn   = raw; break;
			case G_MIDI_IN_VOLUME_OUT:  m.volumeOut  = raw; break;
			case G_MIDI_IN_BEAT_DOUBLE: m.beatDouble = raw; break;
			case G_MIDI_IN_BEAT_HALF:   m.beatHalf   = raw; break;
		}
	});

	stopLearn();
	doneCb();
}


#ifdef WITH_VST

void learnPlugin_(MidiEvent e, std::size_t paramIndex, ID pluginId, std::function<void()> doneCb)
{
	model::onGet(model::plugins, pluginId, [&](Plugin& p)
	{
		assert(paramIndex < p.midiInParams.size());
		p.midiInParams[paramIndex].setValue(e.getRawNoVelocity());
	});

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
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void startChannelLearn(int param, ID channelId, std::function<void()> f)
{
	learnCb_ = [=](m::MidiEvent e) { learnChannel_(e, param, channelId, f); };
}


void startMasterLearn (int param, std::function<void()> f)
{
	learnCb_ = [=](m::MidiEvent e) { learnMaster_(e, param, f); };
}


#ifdef WITH_VST

void startPluginLearn (std::size_t paramIndex, ID pluginId, std::function<void()> f)
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

void clearPluginLearn (std::size_t paramIndex, ID pluginId, std::function<void()> f)
{
	learnPlugin_(MidiEvent(), paramIndex, pluginId, f); // Empty event (0x0)
}

#endif


/* -------------------------------------------------------------------------- */


void dispatch(int byte1, int byte2, int byte3)
{
	/* Here we want to catch two things: a) note on/note off from a keyboard and 
	b) knob/wheel/slider movements from a controller. 
	We must also fix the velocity zero issue for those devices that sends NOTE
	OFF events as NOTE ON + velocity zero. Let's make it a real NOTE OFF event. */

	MidiEvent midiEvent(byte1, byte2, byte3);
	midiEvent.fixVelocityZero();

	u::log::print("[midiDispatcher] MIDI received - 0x%X (chan %d)\n", midiEvent.getRaw(), 
		midiEvent.getChannel());

	/* Start dispatcher. If midi learn is on don't parse channels, just learn 
	incoming MIDI signal. Learn callback wants 'pure' MIDI event, i.e. with
	velocity value stripped off. If midi learn is off process master events first, 
	then each channel in the stack. This way incoming signals don't get processed 
	by glue_* when MIDI learning is on. */

	if (learnCb_ != nullptr) {
		learnCb_(midiEvent);
	}
	else {
		processMaster_(midiEvent);
		processChannels_(midiEvent);
		triggerSignalCb_();
	}	
}


/* -------------------------------------------------------------------------- */


void setSignalCallback(std::function<void()> f)
{
	signalCb_ = f;
}

}}} // giada::m::midiDispatcher::

