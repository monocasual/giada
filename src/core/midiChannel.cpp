/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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
#include "../utils/log.h"
#include "recorder.h"
#include "midiChannelProc.h"
#include "channelManager.h"
#include "channel.h"
#include "recorderHandler.h"
#include "action.h"
#include "patch.h"
#include "const.h"
#include "conf.h"
#include "mixer.h"
#include "pluginHost.h"
#include "kernelMidi.h"
#include "midiChannel.h"


using std::string;


namespace giada {
namespace m 
{
MidiChannel::MidiChannel(int bufferSize)
	: Channel      (ChannelType::MIDI, ChannelStatus::OFF, bufferSize),
		midiOut    (false),
		midiOutChan(G_MIDI_CHANS[0])
{
}


/* -------------------------------------------------------------------------- */


void MidiChannel::copy(const Channel* src_, pthread_mutex_t* pluginMutex)
{
	Channel::copy(src_, pluginMutex);
	const MidiChannel* src = static_cast<const MidiChannel*>(src_);
	midiOut     = src->midiOut;
	midiOutChan = src->midiOutChan;
}


/* -------------------------------------------------------------------------- */


void MidiChannel::parseEvents(mixer::FrameEvents fe)
{
	midiChannelProc::parseEvents(this, fe);
}

/* -------------------------------------------------------------------------- */


void MidiChannel::process(AudioBuffer& out, const AudioBuffer& in, bool audible, 
	bool running)
{
	midiChannelProc::process(this, out, in, audible);
}


/* -------------------------------------------------------------------------- */


void MidiChannel::stopBySeq(bool chansStopOnSeqHalt)
{
	midiChannelProc::stopBySeq(this);
}


/* -------------------------------------------------------------------------- */


void MidiChannel::start(int frame, bool doQuantize, int velocity)
{
	midiChannelProc::start(this);
}


/* -------------------------------------------------------------------------- */


void MidiChannel::kill(int localFrame)
{
	midiChannelProc::kill(this, localFrame);
}


/* -------------------------------------------------------------------------- */


void MidiChannel::rewindBySeq()
{
	midiChannelProc::rewindBySeq(this);
}


/* -------------------------------------------------------------------------- */


void MidiChannel::setMute(bool value)
{
	midiChannelProc::setMute(this, value);
}


/* -------------------------------------------------------------------------- */


void MidiChannel::setSolo(bool value)
{
	midiChannelProc::setSolo(this, value);
}


/* -------------------------------------------------------------------------- */


void MidiChannel::readPatch(const string& basePath, const patch::channel_t& pch)
{
	Channel::readPatch("", pch);
	channelManager::readPatch(this, pch);
}


/* -------------------------------------------------------------------------- */


void MidiChannel::writePatch(int i, bool isProject)
{
	Channel::writePatch(i, isProject);
	channelManager::writePatch(this, isProject, i);
}


/* -------------------------------------------------------------------------- */


#ifdef WITH_VST

void MidiChannel::addVstMidiEvent(uint32_t msg, int localFrame)
{
	juce::MidiMessage message = juce::MidiMessage(
		kernelMidi::getB1(msg),
		kernelMidi::getB2(msg),
		kernelMidi::getB3(msg));
	midiBuffer.addEvent(message, localFrame);
}

#endif


/* -------------------------------------------------------------------------- */


void MidiChannel::empty()
{
	hasActions = false;
}


/* -------------------------------------------------------------------------- */


void MidiChannel::sendMidi(const Action* a, int localFrame)
{
	if (isPlaying() && !mute) {
		if (midiOut) {
			MidiEvent event = a->event;
			event.setChannel(midiOutChan);
			kernelMidi::send(event.getRaw());
		}
#ifdef WITH_VST
		addVstMidiEvent(a->event.getRaw(), localFrame);
#endif
	}
}


/* -------------------------------------------------------------------------- */


void MidiChannel::receiveMidi(const MidiEvent& midiEvent)
{
	namespace mrh = m::recorderHandler;
	namespace mr  = m::recorder;

	if (!armed)
		return;

	/* Now all messages are turned into Channel-0 messages. Giada doesn't care 
	about holding MIDI channel information. Moreover, having all internal 
	messages on channel 0 is way easier. */

	MidiEvent midiEventFlat(midiEvent);
	midiEventFlat.setChannel(0);

#ifdef WITH_VST

	pthread_mutex_lock(&pluginHost::mutex);
	addVstMidiEvent(midiEventFlat.getRaw(), 0);
	pthread_mutex_unlock(&pluginHost::mutex);

#endif

	if (mr::isActive()) {
		mrh::liveRec(index, midiEventFlat);
		hasActions = true;
	}
}


/* -------------------------------------------------------------------------- */


bool MidiChannel::canInputRec()
{
	return false; // midi channels don't handle input audio
}

}} // giada::m::
