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
#include <cstring>
#include "../utils/log.h"
#include "../gui/elems/mainWindow/keyboard/channel.h"
#include "const.h"
#include "channelManager.h"
#include "pluginHost.h"
#include "plugin.h"
#include "kernelMidi.h"
#include "patch.h"
#include "clock.h"
#include "wave.h"
#include "mixer.h"
#include "mixerHandler.h"
#include "conf.h"
#include "patch.h"
#include "waveFx.h"
#include "midiMapConf.h"
#include "channel.h"


using std::string;
using namespace giada::m;


Channel::Channel(int type, int status, int bufferSize)
: bufferSize     (bufferSize),
	volume_i       (1.0f),
	volume_d       (0.0f),
	mute_i         (false),
	guiChannel     (nullptr),
	previewMode    (G_PREVIEW_NONE),
	pan            (0.5f),
	volume         (G_DEFAULT_VOL),
	armed          (false),
	type           (type),
	status         (status),
	key            (0),
	mute           (false),
	mute_s         (false),
	solo           (false),
	hasActions     (false),
	readActions    (false),
	recStatus      (REC_STOPPED),
	midiIn         (true),
	midiInKeyPress (0x0),
	midiInKeyRel   (0x0),
	midiInKill     (0x0),
	midiInArm      (0x0),
	midiInVolume   (0x0),
	midiInMute     (0x0),
	midiInSolo     (0x0),
	midiInFilter   (-1),
	midiOutL       (false),
	midiOutLplaying(0x0),
	midiOutLmute   (0x0),
	midiOutLsolo   (0x0)
{
}


/* -------------------------------------------------------------------------- */


Channel::~Channel()
{
	status = STATUS_OFF;
}


/* -------------------------------------------------------------------------- */


bool Channel::allocBuffers()
{
	if (!vChan.alloc(bufferSize, G_MAX_IO_CHANS)) {
		gu_log("[Channel::allocBuffers] unable to alloc memory for vChan!\n");
		return false;
	}
	return true;
}


/* -------------------------------------------------------------------------- */


void Channel::copy(const Channel* src, pthread_mutex_t* pluginMutex)
{
	key             = src->key;
	volume          = src->volume;
	volume_i        = src->volume_i;
	volume_d        = src->volume_d;
	pan             = src->pan;
	mute_i          = src->mute_i;
	mute_s          = src->mute_s;
	mute            = src->mute;
	solo            = src->solo;
	hasActions      = src->hasActions;
	recStatus       = src->recStatus;
	midiIn          = src->midiIn;
	midiInKeyPress  = src->midiInKeyPress;
	midiInKeyRel    = src->midiInKeyRel;
	midiInKill      = src->midiInKill;
	midiInArm       = src->midiInArm;
	midiInVolume    = src->midiInVolume;
	midiInMute      = src->midiInMute;
	midiInSolo      = src->midiInSolo;
	midiOutL        = src->midiOutL;
	midiOutLplaying = src->midiOutLplaying;
	midiOutLmute    = src->midiOutLmute;
	midiOutLsolo    = src->midiOutLsolo;

	/* clone plugins */

#ifdef WITH_VST
	for (unsigned i=0; i<src->plugins.size(); i++)
		pluginHost::clonePlugin(src->plugins.at(i), pluginHost::CHANNEL,
			pluginMutex, this);
#endif

	/* clone actions */

	for (unsigned i=0; i<recorder::global.size(); i++) {
		for (unsigned k=0; k<recorder::global.at(i).size(); k++) {
			recorder::action* a = recorder::global.at(i).at(k);
			if (a->chan == src->index) {
				recorder::rec(index, a->type, a->frame, a->iValue, a->fValue);
				hasActions = true;
			}
		}
	}
}


/* -------------------------------------------------------------------------- */


void Channel::sendMidiLmessage(uint32_t learn, const midimap::message_t& msg)
{
	gu_log("[channel::sendMidiLmessage] learn=%#X, chan=%d, msg=%#X, offset=%d\n",
		learn, msg.channel, msg.value, msg.offset);

	/* isolate 'channel' from learnt message and offset it as requested by 'nn'
	 * in the midimap configuration file. */

		uint32_t out = ((learn & 0x00FF0000) >> 16) << msg.offset;

	/* merge the previously prepared channel into final message, and finally
	 * send it. */

	out |= msg.value | (msg.channel << 24);
	kernelMidi::send(out);
}


/* -------------------------------------------------------------------------- */


bool Channel::isPlaying() const
{
	return status & (STATUS_PLAY | STATUS_ENDING);
}


/* -------------------------------------------------------------------------- */


void Channel::writePatch(int i, bool isProject)
{
	channelManager::writePatch(this, isProject);
}


/* -------------------------------------------------------------------------- */


void Channel::readPatch(const string& path, int i)
{
	channelManager::readPatch(this, i);
}


/* -------------------------------------------------------------------------- */


void Channel::sendMidiLmute()
{
	if (!midiOutL || midiOutLmute == 0x0)
		return;
	if (mute)
		sendMidiLmessage(midiOutLsolo, midimap::muteOn);
	else
		sendMidiLmessage(midiOutLsolo, midimap::muteOff);
}


/* -------------------------------------------------------------------------- */


void Channel::sendMidiLsolo()
{
	if (!midiOutL || midiOutLsolo == 0x0)
		return;
	if (solo)
		sendMidiLmessage(midiOutLsolo, midimap::soloOn);
	else
		sendMidiLmessage(midiOutLsolo, midimap::soloOff);
}


/* -------------------------------------------------------------------------- */


void Channel::sendMidiLplay()
{
	if (!midiOutL || midiOutLplaying == 0x0)
		return;
	switch (status) {
		case STATUS_OFF:
			sendMidiLmessage(midiOutLplaying, midimap::stopped);
			break;
		case STATUS_PLAY:
			sendMidiLmessage(midiOutLplaying, midimap::playing);
			break;
		case STATUS_WAIT:
			sendMidiLmessage(midiOutLplaying, midimap::waiting);
			break;
		case STATUS_ENDING:
			sendMidiLmessage(midiOutLplaying, midimap::stopping);
	}
}


/* -------------------------------------------------------------------------- */


void Channel::receiveMidi(const MidiEvent& midiEvent)
{
}


/* -------------------------------------------------------------------------- */


bool Channel::isMidiInAllowed(int c) const
{
	return midiInFilter == -1 || midiInFilter == c;
}


/* -------------------------------------------------------------------------- */


void Channel::setPan(float v)
{
	if (v > 1.0f)
		pan = 1.0f;
	else 
	if (v < 0.0f)
		pan = 0.0f;
	else
		pan = v;
}


float Channel::getPan() const
{
	return pan;
}


/* -------------------------------------------------------------------------- */


void Channel::setVolumeI(float v)
{
	volume_i = v;
}


/* -------------------------------------------------------------------------- */


float Channel::calcPanning(int ch)
{
	if (pan == 0.5f) // center: nothing to do
		return 1.0;
	if (ch == 0)
		return 1.0 - pan;
	else  // channel 1
		return pan; 
}


/* -------------------------------------------------------------------------- */


void Channel::setPreviewMode(int m)
{
	previewMode = m;
}


bool Channel::isPreview() const
{
	return previewMode != G_PREVIEW_NONE;
}


/* -------------------------------------------------------------------------- */


#ifdef WITH_VST

juce::MidiBuffer &Channel::getPluginMidiEvents()
{
	return midiBuffer;
}


/* -------------------------------------------------------------------------- */


void Channel::clearMidiBuffer()
{
	midiBuffer.clear();
}


#endif
