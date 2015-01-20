/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * channel
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2014 Giovanni A. Zuliani | Monocasual
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
#include "ge_channel.h"
#include "pluginHost.h"
#include "kernelMidi.h"
#include "patch.h"
#include "wave.h"
#include "mixer.h"
#include "mixerHandler.h"
#include "conf.h"
#include "waveFx.h"
#include "log.h"


extern Patch       G_Patch;
extern Mixer       G_Mixer;
extern Conf        G_Conf;
#ifdef WITH_VST
extern PluginHost  G_PluginHost;
#endif


Channel::Channel(int type, int status, int bufferSize)
	: bufferSize(bufferSize),
	  type      (type),
		status    (status),
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
	  midiIn        (true),
	  midiInKeyPress(0x0),
	  midiInKeyRel  (0x0),
	  midiInKill    (0x0),
	  midiInVolume  (0x0),
	  midiInMute    (0x0),
	  midiInSolo    (0x0),
	  midiOut       (true),
	  midiOutPlaying(0x0),
	  midiOutMute   (0x0),
	  midiOutSolo   (0x0)
{
	vChan = (float *) malloc(bufferSize * sizeof(float));
	if (!vChan)
		gLog("[Channel] unable to alloc memory for vChan\n");
	memset(vChan, 0, bufferSize * sizeof(float));
}


/* ------------------------------------------------------------------ */


Channel::~Channel()
{
	status = STATUS_OFF;
	if (vChan)
		free(vChan);
}


/* ------------------------------------------------------------------ */


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


/* ------------------------------------------------------------------ */


bool Channel::isPlaying()
{
	return status & (STATUS_PLAY | STATUS_ENDING);
}


/* ------------------------------------------------------------------ */


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
}
