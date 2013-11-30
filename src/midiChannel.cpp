/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * channel
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013 Giovanni A. Zuliani | Monocasual
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
 * ------------------------------------------------------------------ */


#include "channel.h"
#include "midiChannel.h"
#include "pluginHost.h"
#include "patch.h"
#include "conf.h"
#include "kernelMidi.h"


extern Patch       G_Patch;
extern Mixer       G_Mixer;
extern Conf        G_Conf;
#ifdef WITH_VST
extern PluginHost  G_PluginHost;
#endif


MidiChannel::MidiChannel(char side)
	: Channel    (CHANNEL_MIDI, STATUS_OFF, side),
	  midiOut    (false),
	  midiOutChan(MIDI_CHANS[0])
{
#ifdef WITH_VST // init VstEvents stack
	freeVstMidiEvents(true);
#endif
}


/* ------------------------------------------------------------------ */


MidiChannel::~MidiChannel() {}


/* ------------------------------------------------------------------ */


#ifdef WITH_VST

void MidiChannel::freeVstMidiEvents(bool init) {
	if (events.numEvents == 0 && !init)
		return;
	memset(events.events, 0, sizeof(VstEvent*) * MAX_VST_EVENTS);
	events.numEvents = 0;
	events.reserved  = 0;
}

#endif


/* ------------------------------------------------------------------ */


#ifdef WITH_VST

void MidiChannel::addVstMidiEvent(uint32_t msg) {
	addVstMidiEvent(G_PluginHost.createVstMidiEvent(msg));
}

#endif


/* ------------------------------------------------------------------ */


#ifdef WITH_VST

void MidiChannel::addVstMidiEvent(VstMidiEvent *e) {
	if (events.numEvents < MAX_VST_EVENTS) {
		events.events[events.numEvents] = (VstEvent*) e;
		events.numEvents++;
		//printf("[MidiChannel] VstMidiEvent added to channel %d, total = %d\n", index, events.numEvents);
	}
	else
		printf("[MidiChannel] channel %d VstEvents = %d > MAX_VST_EVENTS, nothing to do\n", index, events.numEvents);
}

#endif


/* ------------------------------------------------------------------ */


void MidiChannel::onBar(int frame) {}


/* ------------------------------------------------------------------ */


void MidiChannel::stop() {}


/* ------------------------------------------------------------------ */


void MidiChannel::empty() {}


/* ------------------------------------------------------------------ */


void MidiChannel::quantize(int index, int frame) {}


/* ------------------------------------------------------------------ */

#ifdef WITH_VST

VstEvents *MidiChannel::getVstEvents() {
	return (VstEvents *) &events;
}

#endif


/* ------------------------------------------------------------------ */


void MidiChannel::parseAction(recorder::action *a, int localFrame, int globalFrame) {
	if (a->type == ACTION_MIDI)
		sendMidi(a);
}


/* ------------------------------------------------------------------ */


void MidiChannel::onZero(int frame) {
	if (status == STATUS_ENDING)
		status = STATUS_OFF;
	else
	if (status == STATUS_WAIT)
		status = STATUS_PLAY;
}


/* ------------------------------------------------------------------ */


void MidiChannel::setMute(bool internal) {
	mute = true;  	// internal mute does not exist for midi (for now)
	if (midiOut)
		kernelMidi::send(MIDI_ALL_NOTES_OFF);
#ifdef WITH_VST
		addVstMidiEvent(MIDI_ALL_NOTES_OFF);
#endif
}


/* ------------------------------------------------------------------ */


void MidiChannel::unsetMute(bool internal) {
	mute = false;  	// internal mute does not exist for midi (for now)
}


/* ------------------------------------------------------------------ */


void MidiChannel::process(float *buffer, int size) {
#ifdef WITH_VST
	G_PluginHost.processStack(vChan, PluginHost::CHANNEL, this);
	freeVstMidiEvents();
#endif

	for (int j=0; j<size; j+=2) {
		buffer[j]   += vChan[j]   * volume; // * panLeft;   future?
		buffer[j+1] += vChan[j+1] * volume; // * panRight;  future?
	}
}


/* ------------------------------------------------------------------ */


void MidiChannel::start(int frame, bool doQuantize) {
	switch (status) {
		case STATUS_PLAY:
			status = STATUS_ENDING;
			break;
		case STATUS_ENDING:
		case STATUS_WAIT:
			status = STATUS_OFF;
			break;
		case STATUS_OFF:
			status = STATUS_WAIT;
			break;
	}
}


/* ------------------------------------------------------------------ */


void MidiChannel::stopBySeq() {
	kill(0);
}


/* ------------------------------------------------------------------ */


void MidiChannel::kill(int frame) {
	if (status & (STATUS_PLAY | STATUS_ENDING)) {
		if (midiOut)
			kernelMidi::send(MIDI_ALL_NOTES_OFF);
#ifdef WITH_VST
		addVstMidiEvent(MIDI_ALL_NOTES_OFF);
#endif
	}
	status = STATUS_OFF;
}


/* ------------------------------------------------------------------ */


int MidiChannel::loadByPatch(const char *f, int i) {
	volume      = G_Patch.getVol(i);
	index       = G_Patch.getIndex(i);
	mute        = G_Patch.getMute(i);
	mute_s      = G_Patch.getMute_s(i);
	solo        = G_Patch.getSolo(i);
	panLeft     = G_Patch.getPanLeft(i);
	panRight    = G_Patch.getPanRight(i);

	midiOut     = G_Patch.getMidiValue(i, "Out");
	midiOutChan = G_Patch.getMidiValue(i, "OutChan");

	readPatchMidiIn(i);

	return SAMPLE_LOADED_OK;  /// TODO - change name, it's meaningless here
}


/* ------------------------------------------------------------------ */


void MidiChannel::sendMidi(recorder::action *a) {
	if (status & (STATUS_PLAY | STATUS_ENDING) && !mute) {
		if (midiOut)
			kernelMidi::send(a->iValue | MIDI_CHANS[midiOutChan]);
#ifdef WITH_VST
		addVstMidiEvent(a->event);
#endif
	}
}


void MidiChannel::sendMidi(uint32_t data) {
	if (status & (STATUS_PLAY | STATUS_ENDING) && !mute) {
		if (midiOut)
			kernelMidi::send(data | MIDI_CHANS[midiOutChan]);
#ifdef WITH_VST
		addVstMidiEvent(data);
#endif
	}
}


/* ------------------------------------------------------------------ */


void MidiChannel::rewind() {
	if (midiOut)
		kernelMidi::send(MIDI_ALL_NOTES_OFF);
#ifdef WITH_VST
		addVstMidiEvent(MIDI_ALL_NOTES_OFF);
#endif
}


/* ------------------------------------------------------------------ */


void MidiChannel::writePatch(FILE *fp, int i, bool isProject) {
	fprintf(fp, "chanSide%d=%d\n",           i, side);
	fprintf(fp, "chanType%d=%d\n",           i, type);
	fprintf(fp, "chanIndex%d=%d\n",          i, index);
	fprintf(fp, "chanmute%d=%d\n",           i, mute);
	fprintf(fp, "chanMute_s%d=%d\n",         i, mute_s);
	fprintf(fp, "chanSolo%d=%d\n",           i, solo);
	fprintf(fp, "chanvol%d=%f\n",            i, volume);
	fprintf(fp, "chanPanLeft%d=%f\n",        i, panLeft);
	fprintf(fp, "chanPanRight%d=%f\n",       i, panRight);

	/* all values printed as unsigned */

	fprintf(fp, "chanMidiOut%d=%u\n",        i, midiOut);
	fprintf(fp, "chanMidiOutChan%d=%u\n",    i, midiOutChan);

	writePatchMidiIn(fp, i);
}
