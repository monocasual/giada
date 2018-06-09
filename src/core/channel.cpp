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
using namespace giada;
using namespace giada::m;


Channel::Channel(ChannelType type, ChannelStatus status, int bufferSize)
:	guiChannel     (nullptr),
	type           (type),
	status         (status),
	recStatus      (ChannelStatus::OFF),
	previewMode    (PreviewMode::NONE),
	pan            (0.5f),
	volume         (G_DEFAULT_VOL),
	armed          (false),
	key            (0),
	mute           (false),
	solo           (false),
	volume_i       (1.0f),
	volume_d       (0.0f),
	mute_i         (false),
	hasActions     (false),
	readActions    (false),
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
	buffer.alloc(bufferSize, G_MAX_IO_CHANS);
}


/* -------------------------------------------------------------------------- */


void Channel::copy(const Channel* src, pthread_mutex_t* pluginMutex)
{
	using namespace giada::m;

	key             = src->key;
	volume          = src->volume;
	volume_i        = src->volume_i;
	volume_d        = src->volume_d;
	pan             = src->pan;
	mute_i          = src->mute_i;
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


bool Channel::isPlaying() const
{
	return status == ChannelStatus::PLAY || status == ChannelStatus::ENDING;
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
		kernelMidi::sendMidiLightning(midiOutLmute, midimap::muteOn);
	else
		kernelMidi::sendMidiLightning(midiOutLmute, midimap::muteOff);
}


/* -------------------------------------------------------------------------- */


void Channel::sendMidiLsolo()
{
	if (!midiOutL || midiOutLsolo == 0x0)
		return;
	if (solo)
		kernelMidi::sendMidiLightning(midiOutLsolo, midimap::soloOn);
	else
		kernelMidi::sendMidiLightning(midiOutLsolo, midimap::soloOff);
}


/* -------------------------------------------------------------------------- */


void Channel::sendMidiLstatus()
{
	if (!midiOutL || midiOutLplaying == 0x0)
		return;
	switch (status) {
		case ChannelStatus::OFF:
			kernelMidi::sendMidiLightning(midiOutLplaying, midimap::stopped);
			break;
		case ChannelStatus::PLAY:
			kernelMidi::sendMidiLightning(midiOutLplaying, midimap::playing);
			break;
		case ChannelStatus::WAIT:
			kernelMidi::sendMidiLightning(midiOutLplaying, midimap::waiting);
			break;
		case ChannelStatus::ENDING:
			kernelMidi::sendMidiLightning(midiOutLplaying, midimap::stopping);
			break;
		default:
			break;
	}
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


float Channel::calcPanning(int ch) const
{
	if (pan == 0.5f) // center: nothing to do
		return 1.0;
	if (ch == 0)
		return 1.0 - pan;
	else  // channel 1
		return pan; 
}


/* -------------------------------------------------------------------------- */


void Channel::calcVolumeEnvelope()
{
	volume_i += volume_d;
	if (volume_i < 0.0f)
		volume_i = 0.0f;
	else
	if (volume_i > 1.0f)
		volume_i = 1.0f;	
}


bool Channel::isPreview() const
{
	return previewMode != PreviewMode::NONE;
}


/* -------------------------------------------------------------------------- */


bool Channel::isReadingActions() const
{
	return hasActions && readActions;
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
