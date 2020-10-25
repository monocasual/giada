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

#include <functional>
#include <map>
#include <cmath>
#include "utils/log.h"
#include "core/model/model.h"

#include "core/midiLearner.h"
#include "core/midiDispatcher.h"
#include "core/midiPorts.h"
#include "core/midiMsg.h"

namespace giada {
namespace m {
namespace midiLearner
{
namespace
{
// Remembers values of first occurences of CC, so it's easier to tell
// which one changed and how much
std::map<MidiMsg, int> ccMon;

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


void learnChannel_(MidiEvent e, int param, ID channelId, std::function<void()> doneCb)
{
	if (!isChannelMidiInAllowed_(channelId, e.getChannel()))
		return;	

	uint32_t raw = e.getRawNoVelocity();

	model::onGet(model::channels, channelId, [param, raw](Channel& c)
	{	
		switch (param) {
			case G_MIDI_IN_KEYPRESS:     c.midiLearner.state->keyPress.store(raw);    break;
			case G_MIDI_IN_KEYREL:       c.midiLearner.state->keyRelease.store(raw);  break;
			case G_MIDI_IN_KILL:         c.midiLearner.state->kill.store(raw);        break;
			case G_MIDI_IN_ARM:          c.midiLearner.state->arm.store(raw);         break;
			case G_MIDI_IN_MUTE:         c.midiLearner.state->mute.store(raw);        break;
			case G_MIDI_IN_SOLO:         c.midiLearner.state->solo.store(raw);        break;
			case G_MIDI_IN_VOLUME:       c.midiLearner.state->volume.store(raw);      break;
			case G_MIDI_IN_PITCH:        c.midiLearner.state->pitch.store(raw);       break;
			case G_MIDI_IN_READ_ACTIONS: c.midiLearner.state->readActions.store(raw); break;
			case G_MIDI_OUT_L_PLAYING:   c.midiLighter.state->playing.store(raw);     break;
			case G_MIDI_OUT_L_MUTE:      c.midiLighter.state->mute.store(raw);        break;
			case G_MIDI_OUT_L_SOLO:      c.midiLighter.state->solo.store(raw);        break;
		}
	});

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
	doneCb();
}


#ifdef WITH_VST

void learnPlugin_(MidiEvent e, int paramIndex, ID pluginId, std::function<void()> doneCb)
{
	model::onSwap(model::plugins, pluginId, [&](Plugin& p)
	{
		p.midiInParams[paramIndex] = e.getRawNoVelocity();
	});

	doneCb();
}

#endif

} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void startChannelLearn(int param, ID channelId, std::function<void()> f)
{
	learnCb_ = [=](m::MidiEvent e) { learnChannel_(e, param, channelId, f); };
	midiDispatcher::registerExRule(midiPorts::getInPorts(1),
			MMF_NOTEONOFFCC,
			"m;midiLearner");
}


void startMasterLearn (int param, std::function<void()> f)
{
	learnCb_ = [=](m::MidiEvent e) { learnMaster_(e, param, f); };
	midiDispatcher::registerExRule(midiPorts::getInPorts(1),
			MMF_NOTEONOFFCC,
			"m;midiLearner");
}


#ifdef WITH_VST

void startPluginLearn (int paramIndex, ID pluginId, std::function<void()> f)
{
	learnCb_ = [=](m::MidiEvent e) { learnPlugin_(e, paramIndex, pluginId, f); };
	midiDispatcher::registerExRule(midiPorts::getInPorts(1),
			MMF_NOTEONOFFCC,
			"m;midiLearner");
}

#endif


void stopLearn()
{
	midiDispatcher::unregisterExRule("m;midiLearner");
	learnCb_ = nullptr;
	ccMon.clear();
	u::log::print("[ML::stopLearn] Done.\n");
}


/* -------------------------------------------------------------------------- */


void clearMasterLearn(int param, std::function<void()> f)
{
	midiDispatcher::unregisterExRule("m;midiLearner");
	learnMaster_(MidiEvent(), param, f); // Empty event (0x0)
}


void clearChannelLearn(int param, ID channelId, std::function<void()> f)
{
	midiDispatcher::unregisterExRule("m;midiLearner");
	learnChannel_(MidiEvent(), param, channelId, f); // Empty event (0x0)
}


#ifdef WITH_VST

void clearPluginLearn (int paramIndex, ID pluginId, std::function<void()> f)
{
	midiDispatcher::unregisterExRule("m;midiLearner");
	learnPlugin_(MidiEvent(), paramIndex, pluginId, f); // Empty event (0x0)
}

#endif


/* -------------------------------------------------------------------------- */


void midiReceive(const MidiMsg& mm) 
{
	if (learnCb_ == nullptr) {
		stopLearn();
		u::log::print("[ML::midiReceive] No learnCb_ set! Aborting.\n");
		return;
	}

	// CCs are accepted only if their value changed by 32 or more.
	// The purpose is to filter out possible pot jitter
	// and to make it easier to learn coupled controllers, like XY pads

	MidiMsg noval = mm.noValue();
	if (MMF_CC << mm) {
		if (ccMon.count(noval) == 0) {
			ccMon[noval] = mm.getValue();
			return;
		}
		else {
			if (std::abs(ccMon[noval] - (int)mm.getValue()) <= 32) {
				return;
			}
		}

	}

	MidiEvent midiEvent(mm.getByte(0), mm.getByte(1), mm.getByte(2));
	learnCb_(midiEvent);
	stopLearn();
}

}}} // giada::m::midiLearner::

