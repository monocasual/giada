/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * channel
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2015 Giovanni A. Zuliani | Monocasual
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


#include "channel.h"
#include "pluginHost.h"
#include "kernelMidi.h"
#include "patch.h"
#include "wave.h"
#include "mixer.h"
#include "mixerHandler.h"
#include "conf.h"
#include "waveFx.h"
#include "log.h"
#include "midiMapConf.h"
#include "ge_channel.h"


extern Patch       G_Patch;
extern Mixer       G_Mixer;
extern Conf        G_Conf;
extern MidiMapConf G_MidiMap;
#ifdef WITH_VST
extern PluginHost  G_PluginHost;
#endif


Channel::Channel(int type, int status, int bufferSize)
	: bufferSize(bufferSize),
	  type      (type),
		status    (status),
		key       (0),
	  volume    (DEFAULT_VOL),
	  volume_i  (1.0f),
	  volume_d  (0.0f),
	  panLeft   (1.0f),
	  panRight  (1.0f),
	  mute_i    (false),
	  mute_s    (false),
	  mute      (false),
	  solo      (false),
	  hasActions(false),
	  recStatus (REC_STOPPED),
	  vChan     (NULL),
	  guiChannel(NULL),
	  midiIn         (true),
	  midiInKeyPress (0x0),
	  midiInKeyRel   (0x0),
	  midiInKill     (0x0),
	  midiInVolume   (0x0),
	  midiInMute     (0x0),
	  midiInSolo     (0x0),
	  midiOutL       (true),
	  midiOutLplaying(0x0),
	  midiOutLmute   (0x0),
	  midiOutLsolo   (0x0)
{
	vChan = (float *) malloc(bufferSize * sizeof(float));
	if (!vChan)
		gLog("[Channel] unable to alloc memory for vChan\n");
	memset(vChan, 0, bufferSize * sizeof(float));
}


/* -------------------------------------------------------------------------- */


Channel::~Channel()
{
	status = STATUS_OFF;
	if (vChan)
		free(vChan);
}


/* -------------------------------------------------------------------------- */


void Channel::readPatchMidiIn(int i)
{
	midiIn         = G_Patch.getMidiValue(i, "In");
	midiInKeyPress = G_Patch.getMidiValue(i, "InKeyPress");
	midiInKeyRel   = G_Patch.getMidiValue(i, "InKeyRel");
  midiInKill     = G_Patch.getMidiValue(i, "InKill");
  midiInVolume   = G_Patch.getMidiValue(i, "InVolume");
  midiInMute     = G_Patch.getMidiValue(i, "InMute");
  midiInSolo     = G_Patch.getMidiValue(i, "InSolo");
}

void Channel::readPatchMidiOut(int i)
{
	midiOutL        = G_Patch.getMidiValue(i, "OutL");
	midiOutLplaying = G_Patch.getMidiValue(i, "OutLplaying");
	midiOutLmute    = G_Patch.getMidiValue(i, "OutLmute");
	midiOutLsolo    = G_Patch.getMidiValue(i, "OutLsolo");
}


/* -------------------------------------------------------------------------- */


bool Channel::isPlaying()
{
	return status & (STATUS_PLAY | STATUS_ENDING);
}


/* -------------------------------------------------------------------------- */


void Channel::writePatch(FILE *fp, int i, bool isProject)
{
	fprintf(fp, "chanType%d=%d\n",     i, type);
	fprintf(fp, "chanIndex%d=%d\n",    i, index);
	fprintf(fp, "chanColumn%d=%d\n",   i, guiChannel->getColumnIndex());
	fprintf(fp, "chanMute%d=%d\n",     i, mute);
	fprintf(fp, "chanMute_s%d=%d\n",   i, mute_s);
	fprintf(fp, "chanSolo%d=%d\n",     i, solo);
	fprintf(fp, "chanvol%d=%f\n",      i, volume);
	fprintf(fp, "chanPanLeft%d=%f\n",  i, panLeft);
	fprintf(fp, "chanPanRight%d=%f\n", i, panRight);

	fprintf(fp, "chanMidiIn%d=%u\n",         i, midiIn);
	fprintf(fp, "chanMidiInKeyPress%d=%u\n", i, midiInKeyPress);
	fprintf(fp, "chanMidiInKeyRel%d=%u\n",   i, midiInKeyRel);
	fprintf(fp, "chanMidiInKill%d=%u\n",     i, midiInKill);
	fprintf(fp, "chanMidiInVolume%d=%u\n",   i, midiInVolume);
	fprintf(fp, "chanMidiInMute%d=%u\n",     i, midiInMute);
	fprintf(fp, "chanMidiInSolo%d=%u\n",     i, midiInSolo);

	fprintf(fp, "chanMidiOutL%d=%u\n",        i, midiOutL);
	fprintf(fp, "chanMidiOutLplaying%d=%u\n", i, midiOutLplaying);
	fprintf(fp, "chanMidiOutLmute%d=%u\n",    i, midiOutLmute);
	fprintf(fp, "chanMidiOutLsolo%d=%u\n",    i, midiOutLsolo);
}


/* -------------------------------------------------------------------------- */


void Channel::sendMidiLmute()
{
	if (!midiOutL || midiOutLmute == 0x0)
		return;

	gLog("[channel] send MIDI lightning 'mute' - \n");

	if (mute)
		kernelMidi::midi_gen_messages(midiOutLmute, G_MidiMap.mute_on_channel, G_MidiMap.mute_on, G_MidiMap.mute_on_notePos);
	else
		kernelMidi::midi_gen_messages(midiOutLmute, G_MidiMap.mute_off_channel, G_MidiMap.mute_off, G_MidiMap.mute_off_notePos);
}


/* -------------------------------------------------------------------------- */


void Channel::sendMidiLsolo()
{
	if (!midiOutL || midiOutLsolo == 0x0)
		return;

	gLog("[channel] send MIDI lightning 'solo' - \n");

	if (solo)
		kernelMidi::midi_gen_messages(midiOutLsolo, G_MidiMap.solo_on_channel, G_MidiMap.solo_on, G_MidiMap.solo_on_notePos);
	else
		kernelMidi::midi_gen_messages(midiOutLsolo, G_MidiMap.solo_off_channel, G_MidiMap.solo_off, G_MidiMap.solo_off_notePos);
}


/* -------------------------------------------------------------------------- */


void Channel::sendMidiLplay()
{
	if (!midiOutL || midiOutLplaying == 0x0)
		return;

	gLog("[channel] send MIDI lightning 'playing' - ");

	switch (status) {
		case STATUS_OFF:
			gLog("STATUS_OFF - learn=%#x, chan=%d\n",
					midiOutLplaying, G_MidiMap.stopped_channel);
			kernelMidi::midi_gen_messages(midiOutLplaying, G_MidiMap.stopped_channel, G_MidiMap.stopped, G_MidiMap.stopped_notePos);
			break;
		case STATUS_PLAY:
			gLog("STATUS_PLAY - learn=%#x, chan=%d\n",
					midiOutLplaying, G_MidiMap.playing_channel);
			kernelMidi::midi_gen_messages(midiOutLplaying, G_MidiMap.playing_channel, G_MidiMap.playing, G_MidiMap.playing_notePos);
			break;
		case STATUS_WAIT:
			gLog("STATUS_WAIT - learn=%#x, chan=%d\n",
					midiOutLplaying, G_MidiMap.waiting_channel);
			kernelMidi::midi_gen_messages(midiOutLplaying, G_MidiMap.waiting_channel, G_MidiMap.waiting, G_MidiMap.waiting_notePos);
			break;
		case STATUS_ENDING:
			gLog("STATUS_ENDING - learn=%#x, chan=%d\n",
					midiOutLplaying, G_MidiMap.stopping_channel);
			kernelMidi::midi_gen_messages(midiOutLplaying, G_MidiMap.stopping_channel, G_MidiMap.stopping, G_MidiMap.stopping_notePos);
	}
}
