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
#include "utils/log.h"
#include "core/channels/midiChannelProc.h"
#include "core/channels/channelManager.h"
#include "core/channels/channel.h"
#include "core/recorder.h"
#include "core/recorderHandler.h"
#include "core/recManager.h"
#include "core/action.h"
#include "core/patch.h"
#include "core/const.h"
#include "core/conf.h"
#include "core/mixer.h"
#include "core/pluginHost.h"
#include "core/kernelMidi.h"
#include "midiChannel.h"


namespace giada {
namespace m 
{
MidiChannel::MidiChannel(int bufferSize, size_t column)
: Channel    (ChannelType::MIDI, ChannelStatus::OFF, bufferSize, column),
  midiOut    (false),
  midiOutChan(G_MIDI_CHANS[0])
{
}


/* -------------------------------------------------------------------------- */


MidiChannel::MidiChannel(const MidiChannel& o)
: Channel    (o),
  midiOut    (o.midiOut),
  midiOutChan(o.midiOutChan)
{
}


/* -------------------------------------------------------------------------- */


MidiChannel* MidiChannel::clone() const
{
	return new MidiChannel(*this);
}


/* -------------------------------------------------------------------------- */


void MidiChannel::parseEvents(mixer::FrameEvents fe)
{
	midiChannelProc::parseEvents(this, fe);
}

/* -------------------------------------------------------------------------- */


void MidiChannel::render(AudioBuffer& out, const AudioBuffer& in, 
	AudioBuffer& inToOut, bool audible, bool running)
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


void MidiChannel::readPatch(const std::string& basePath, const patch::channel_t& pch)
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


void MidiChannel::empty()
{
	hasActions = false;
}


/* -------------------------------------------------------------------------- */


void MidiChannel::sendMidi(const MidiEvent& e, int localFrame)
{
	if (midiOut) {
		MidiEvent e_ = e;
		e_.setChannel(midiOutChan);
		kernelMidi::send(e_.getRaw());
	}

#ifdef WITH_VST

	/* Enqueue this MIDI event for plug-ins processing. Will be read and
	rendered later on by the audio thread. */

	MidiEvent e_ = e;
	e_.setDelta(localFrame);
	midiQueue.push(e_);

#endif
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

	/* Enqueue this MIDI event for plug-ins processing. Will be read and
	rendered later on by the audio thread. */

	midiQueue.push(midiEventFlat);

#endif

	if (recManager::isRecordingAction()) {
		mrh::liveRec(id, midiEventFlat);
		hasActions = true;
	}
}

}} // giada::m::
