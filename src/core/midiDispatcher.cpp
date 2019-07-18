/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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
#include "glue/io.h"
#include "glue/channel.h"
#include "glue/main.h"
#include "utils/log.h"
#include "utils/math.h"
#include "core/model/model.h"
#include "core/channels/channel.h"
#include "core/channels/sampleChannel.h"
#include "core/channels/midiChannel.h"
#include "core/conf.h"
#include "core/mixer.h"
#include "core/mixerHandler.h"
#include "core/pluginHost.h"
#include "core/plugin.h"
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


#ifdef WITH_VST

void processPlugins_(const std::vector<ID>& ids, const MidiEvent& midiEvent)
{
	uint32_t pure = midiEvent.getRawNoVelocity();
	float    vf   = u::math::map(midiEvent.getVelocity(), G_MAX_VELOCITY, 1.0f);

	/* Plugins' parameters layout reflects the structure of the matrix
	Channel::midiInPlugins. It is safe to assume then that Plugin 'p' and 
	k indexes match both the structure of Channel::midiInPlugins and the vector
	of plugins. */

	m::model::PluginsLock l(m::model::plugins);

	for (ID id : ids) {
		m::Plugin& p = m::model::get(m::model::plugins, id);
		for (unsigned k = 0; k < p.midiInParams.size(); k++) {
			if (pure != p.midiInParams.at(k))
				continue;
			c::plugin::setParameter(id, k, vf, /*gui=*/false);
			gu_log("  >>> [plugin %d parameter %d] (pure=0x%X, value=%d, float=%f)\n",
				p.id, k, pure, midiEvent.getVelocity(), vf);
		}
	}
}

#endif


/* -------------------------------------------------------------------------- */


void processChannels_(const MidiEvent& midiEvent)
{
	uint32_t pure = midiEvent.getRawNoVelocity();

	/* TODO - this is definitely not the best approach but it's necessary as
	you can't call actions on m::model::channels while locking on a upper
	level. Let's wait for a better async mechanism... */

	std::vector<std::function<void()>> actions;

	m::model::channels.lock();
	for (Channel* ch : m::model::channels) {

		/* Do nothing on this channel if MIDI in is disabled or filtered out for
		the current MIDI channel. */

		if (!ch->midiIn || !ch->isMidiInAllowed(midiEvent.getChannel()))
			continue;

		if      (pure == ch->midiInKeyPress) {
			actions.push_back([&] {
				gu_log("  >>> keyPress, ch=%d (pure=0x%X)\n", ch->id, pure);
				c::io::keyPress(ch->id, false, false, midiEvent.getVelocity());
			});
		}
		else if (pure == ch->midiInKeyRel) {
			actions.push_back([&] {
				gu_log("  >>> keyRel ch=%d (pure=0x%X)\n", ch->id, pure);
				c::io::keyRelease(ch->id, false, false);
			});
		}
		else if (pure == ch->midiInMute) {
			actions.push_back([&] {
				gu_log("  >>> mute ch=%d (pure=0x%X)\n", ch->id, pure);
				c::channel::toggleMute(ch->id);
			});
		}		
		else if (pure == ch->midiInKill) {
			actions.push_back([&] {
				gu_log("  >>> kill ch=%d (pure=0x%X)\n", ch->id, pure);
				c::channel::kill(ch->id, /*record=*/false);
			});
		}		
		else if (pure == ch->midiInArm) {
			actions.push_back([&] {
				gu_log("  >>> arm ch=%d (pure=0x%X)\n", ch->id, pure);
				c::channel::toggleArm(ch->id);
			});
		}
		else if (pure == ch->midiInSolo) {
			actions.push_back([&] {
				gu_log("  >>> solo ch=%d (pure=0x%X)\n", ch->id, pure);
				c::channel::toggleSolo(ch->id);
			});
		}
		else if (pure == ch->midiInVolume) {
			actions.push_back([&] {
				float vf = u::math::map(midiEvent.getVelocity(), G_MAX_VELOCITY, G_MAX_VOLUME); 
				gu_log("  >>> volume ch=%d (pure=0x%X, value=%d, float=%f)\n",
					ch->id, pure, midiEvent.getVelocity(), vf);
				c::channel::setVolume(ch->id, vf, /*gui=*/false);
			});
		}
		else {
			const SampleChannel* sch = static_cast<const SampleChannel*>(ch);
			if (pure == sch->midiInPitch) {
				actions.push_back([&] {
					float vf = u::math::map(midiEvent.getVelocity(), G_MAX_VELOCITY, G_MAX_PITCH); 
					gu_log("  >>> pitch ch=%d (pure=0x%X, value=%d, float=%f)\n",
						sch->id, pure, midiEvent.getVelocity(), vf);
					c::channel::setPitch(sch->id, vf);
				});
			}
			else 
			if (pure == sch->midiInReadActions) {
				actions.push_back([&] {
					gu_log("  >>> toggle read actions ch=%d (pure=0x%X)\n", sch->id, pure);
					c::channel::toggleReadingActions(sch->id);
				});
			}
		}
#ifdef WITH_VST

		/* Process learned plugins parameters. */
		processPlugins_(ch->pluginIds, midiEvent); 

#endif

		/* Redirect full midi message (pure + velocity) to plugins. */
		ch->receiveMidi(midiEvent.getRaw());
	}
	m::model::channels.unlock();

	/* Apply all the collected actions. */
	for (auto& action : actions)
		action();
}


/* -------------------------------------------------------------------------- */


void processMaster_(const MidiEvent& midiEvent)
{
	const bool gui = false;

	uint32_t pure = midiEvent.getRawNoVelocity();

	if      (pure == conf::midiInRewind) {
		gu_log("  >>> rewind (master) (pure=0x%X)\n", pure);
		mh::rewindSequencer();
	}
	else if (pure == conf::midiInStartStop) {
		gu_log("  >>> startStop (master) (pure=0x%X)\n", pure);
		mh::toggleSequencer();
	}
	else if (pure == conf::midiInActionRec) {
		gu_log("  >>> actionRec (master) (pure=0x%X)\n", pure);
		recManager::toggleActionRec(static_cast<RecTriggerMode>(conf::recTriggerMode));
	}
	else if (pure == conf::midiInInputRec) {
		gu_log("  >>> inputRec (master) (pure=0x%X)\n", pure);
		c::main::toggleInputRec();
	}
	else if (pure == conf::midiInMetronome) {
		gu_log("  >>> metronome (master) (pure=0x%X)\n", pure);
		m::mixer::toggleMetronome();
	}
	else if (pure == conf::midiInVolumeIn) {
		float vf = u::math::map(midiEvent.getVelocity(), G_MAX_VELOCITY, G_MAX_VOLUME); 
		gu_log("  >>> input volume (master) (pure=0x%X, value=%d, float=%f)\n",
			pure, midiEvent.getVelocity(), vf);
		c::main::setInVol(vf, gui);
	}
	else if (pure == conf::midiInVolumeOut) {
		float vf = u::math::map(midiEvent.getVelocity(), G_MAX_VELOCITY, G_MAX_VOLUME); 
		gu_log("  >>> output volume (master) (pure=0x%X, value=%d, float=%f)\n",
			pure, midiEvent.getVelocity(), vf);
		c::main::setOutVol(vf, gui);
	}
	else if (pure == conf::midiInBeatDouble) {
		gu_log("  >>> sequencer x2 (master) (pure=0x%X)\n", pure);
		c::main::beatsMultiply();
	}
	else if (pure == conf::midiInBeatHalf) {
		gu_log("  >>> sequencer /2 (master) (pure=0x%X)\n", pure);
		c::main::beatsDivide();
	}
}


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


void startMidiLearn(std::function<void(MidiEvent)> f)
{
	learnCb_ = f;
}


/* -------------------------------------------------------------------------- */


void stopMidiLearn()
{
	learnCb_ = nullptr;
}


/* -------------------------------------------------------------------------- */


void dispatch(int byte1, int byte2, int byte3)
{
	/* Here we want to catch two things: a) note on/note off from a keyboard and 
	b) knob/wheel/slider movements from a controller. 
	We must also fix the velocity zero issue for those devices that sends NOTE
	OFF events as NOTE ON + velocity zero. Let's make it a real NOTE OFF event. */

	MidiEvent midiEvent(byte1, byte2, byte3);
	midiEvent.fixVelocityZero();

	gu_log("[midiDispatcher] MIDI received - 0x%X (chan %d)\n", midiEvent.getRaw(), 
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

}}}; // giada::m::midiDispatcher::

