/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#include "../utils/log.h"
#include "midiChannel.h"
#include "channel.h"
#include "patch.h"
#include "const.h"
#include "clock.h"
#include "conf.h"
#include "mixer.h"
#ifdef WITH_VST
  #include "pluginHost.h"
#endif
#include "kernelMidi.h"


using std::string;
using namespace giada;


MidiChannel::MidiChannel(int bufferSize)
	: Channel    (CHANNEL_MIDI, STATUS_OFF, bufferSize),
	  midiOut    (false),
	  midiOutChan(MIDI_CHANS[0])
{
}


/* -------------------------------------------------------------------------- */


MidiChannel::~MidiChannel() {}


/* -------------------------------------------------------------------------- */


void MidiChannel::copy(const Channel *_src, pthread_mutex_t *pluginMutex)
{
	Channel::copy(_src, pluginMutex);
	MidiChannel *src = (MidiChannel *) _src;
	midiOut     = src->midiOut;
	midiOutChan = src->midiOutChan;
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


void MidiChannel::onBar(int frame) {}


/* -------------------------------------------------------------------------- */


void MidiChannel::stop() {}


/* -------------------------------------------------------------------------- */


void MidiChannel::empty() {}


/* -------------------------------------------------------------------------- */


void MidiChannel::quantize(int index, int localFrame) {}


/* -------------------------------------------------------------------------- */


void MidiChannel::parseAction(recorder::action *a, int localFrame,
		int globalFrame, int quantize, bool mixerIsRunning)
{
	if (a->type == G_ACTION_MIDI)
		sendMidi(a, localFrame/2);
}


/* -------------------------------------------------------------------------- */


void MidiChannel::onZero(int frame, bool recsStopOnChanHalt)
{
	if (status == STATUS_ENDING) {
		status = STATUS_OFF;
		sendMidiLplay();
	}
	else
	if (status == STATUS_WAIT) {
		status = STATUS_PLAY;
		sendMidiLplay();
	}
}


/* -------------------------------------------------------------------------- */


void MidiChannel::setMute(bool internal)
{
	mute = true;  	// internal mute does not exist for midi (for now)
	if (midiOut)
		kernelMidi::send(MIDI_ALL_NOTES_OFF);
#ifdef WITH_VST
		addVstMidiEvent(MIDI_ALL_NOTES_OFF, 0);
#endif
	sendMidiLmute();
}


/* -------------------------------------------------------------------------- */


void MidiChannel::unsetMute(bool internal)
{
	mute = false;  	// internal mute does not exist for midi (for now)
	sendMidiLmute();
}


/* -------------------------------------------------------------------------- */


void MidiChannel::process(float *outBuffer, float *inBuffer)
{
#ifdef WITH_VST
	pluginHost::processStack(vChan, pluginHost::CHANNEL, this);
#endif

	/* TODO - isn't this useful only if WITH_VST ? */
	for (int j=0; j<bufferSize; j+=2) {
		outBuffer[j]   += vChan[j]   * volume; // * panLeft;   future?
		outBuffer[j+1] += vChan[j+1] * volume; // * panRight;  future?
	}
}


/* -------------------------------------------------------------------------- */


void MidiChannel::start(int frame, bool doQuantize, int quantize,
		bool mixerIsRunning, bool forceStart, bool isUserGenerated)
{
	switch (status) {
		case STATUS_PLAY:
			status = STATUS_ENDING;
			sendMidiLplay();
			break;
		case STATUS_ENDING:
		case STATUS_WAIT:
			status = STATUS_OFF;
			sendMidiLplay();
			break;
		case STATUS_OFF:
			status = STATUS_WAIT;
			sendMidiLplay();
			break;
	}
}


/* -------------------------------------------------------------------------- */


void MidiChannel::stopBySeq(bool chansStopOnSeqHalt)
{
	kill(0);
}


/* -------------------------------------------------------------------------- */


void MidiChannel::kill(int frame)
{
	if (status & (STATUS_PLAY | STATUS_ENDING)) {
		if (midiOut)
			kernelMidi::send(MIDI_ALL_NOTES_OFF);
#ifdef WITH_VST
		addVstMidiEvent(MIDI_ALL_NOTES_OFF, 0);
#endif
	}
	status = STATUS_OFF;
	sendMidiLplay();
}


/* -------------------------------------------------------------------------- */


int MidiChannel::readPatch(const string &basePath, int i,
		pthread_mutex_t *pluginMutex, int samplerate, int rsmpQuality)
{
	Channel::readPatch("", i, pluginMutex, samplerate, rsmpQuality);

	patch::channel_t *pch = &patch::channels.at(i);

	midiOut     = pch->midiOut;
	midiOutChan = pch->midiOutChan;

	return SAMPLE_LOADED_OK;  /// TODO - change name, it's meaningless here
}


/* -------------------------------------------------------------------------- */


void MidiChannel::sendMidi(recorder::action *a, int localFrame)
{
	if (status & (STATUS_PLAY | STATUS_ENDING) && !mute) {
		if (midiOut)
			kernelMidi::send(a->iValue | MIDI_CHANS[midiOutChan]);

#ifdef WITH_VST
		addVstMidiEvent(a->iValue, localFrame);
#endif
	}
}


void MidiChannel::sendMidi(uint32_t data)
{
	if (status & (STATUS_PLAY | STATUS_ENDING) && !mute) {
		if (midiOut)
			kernelMidi::send(data | MIDI_CHANS[midiOutChan]);
#ifdef WITH_VST
		addVstMidiEvent(data, 0);
#endif
	}
}


/* -------------------------------------------------------------------------- */


void MidiChannel::rewind()
{
	if (midiOut)
		kernelMidi::send(MIDI_ALL_NOTES_OFF);
#ifdef WITH_VST
		addVstMidiEvent(MIDI_ALL_NOTES_OFF, 0);
#endif
}


/* -------------------------------------------------------------------------- */


int MidiChannel::writePatch(int i, bool isProject)
{
	int pchIndex = Channel::writePatch(i, isProject);
	patch::channel_t *pch = &patch::channels.at(pchIndex);

	pch->midiOut     = midiOut;
	pch->midiOutChan = midiOutChan;

	return 0;
}


/* -------------------------------------------------------------------------- */


void MidiChannel::receiveMidi(uint32_t msg)
{
  if (!armed)
    return;

  /* Filter time, based on MIDI channel. If midiFilter == -1, all messages
  are taken into account. */
  /* TODO - actual MIDI filtering not yet implemented. */

  if (midiFilter != -1)
  {
    gu_log("[Channel::processMidi] MIDI channel filtering not yet implemented\n");
    return;
  }

  /* Now all messages are turned into Channel-0 messages. Giada doesn't care about
  holding MIDI channel information. Moreover, having all internal messages on
  channel 0 is way easier. */

  msg = kernelMidi::setChannel(msg, 0);

#ifdef WITH_VST

  while (true) {
    if (pthread_mutex_trylock(&pluginHost::mutex_midi) != 0)
      continue;
    gu_log("[Channel::processMidi] msg=%X\n", msg);
    addVstMidiEvent(msg, 0);
    pthread_mutex_unlock(&pluginHost::mutex_midi);
    break;
  }

#endif

	if (recorder::canRec(this, clock::isRunning(), mixer::recording)) {
		recorder::rec(index, G_ACTION_MIDI, clock::getCurrentFrame(), msg);
    hasActions = true;
  }
}


/* -------------------------------------------------------------------------- */


bool MidiChannel::canInputRec()
{
	return false; // midi channels don't handle input audio
}


/* -------------------------------------------------------------------------- */


void MidiChannel::clear() {}
