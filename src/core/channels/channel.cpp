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
#include "core/channels/channelManager.h"
#include "core/const.h"
#include "core/pluginManager.h"
#include "core/plugin.h"
#include "core/kernelMidi.h"
#include "core/patch.h"
#include "core/clock.h"
#include "core/wave.h"
#include "core/mixer.h"
#include "core/mixerHandler.h"
#include "core/recorderHandler.h"
#include "core/conf.h"
#include "core/patch.h"
#include "core/waveFx.h"
#include "core/midiMapConf.h"
#include "channel.h"


namespace giada {
namespace m 
{
Channel::Channel(ChannelType type, ChannelStatus playStatus, int bufferSize, 
	ID columnId, ID id)
: type           (type),
  playStatus     (playStatus),
  recStatus      (ChannelStatus::OFF),
  columnId       (columnId),
  id             (id),
  previewMode    (PreviewMode::NONE),
  pan            (0.5f),
  volume         (G_DEFAULT_VOL),
  armed          (false),
  key            (0),
  mute           (false),
  solo           (false),
  volume_i       (1.0f),
  volume_d       (0.0f),
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

#ifdef WITH_VST

	midiBuffer.ensureSize(bufferSize);

#endif
}


/* -------------------------------------------------------------------------- */


Channel::Channel(const Channel& o)
: type           (o.type),
  playStatus     (o.playStatus),
  recStatus      (o.recStatus),
  columnId       (o.columnId),
  id             (o.id),
  previewMode    (o.previewMode),
  pan            (o.pan),
  volume         (o.volume),
  armed          (o.armed),
  name           (o.name),
  key            (o.key),
  mute           (o.mute),
  solo           (o.solo),
  volume_i       (o.volume_i.load()),
  volume_d       (o.volume_d),
  hasActions     (o.hasActions),
  readActions    (o.readActions),
  midiIn         (o.midiIn.load()),
  midiInKeyPress (o.midiInKeyPress.load()),
  midiInKeyRel   (o.midiInKeyRel.load()),
  midiInKill     (o.midiInKill.load()),
  midiInArm      (o.midiInArm.load()),
  midiInVolume   (o.midiInVolume.load()),
  midiInMute     (o.midiInMute.load()),
  midiInSolo     (o.midiInSolo.load()),
  midiInFilter   (o.midiInFilter.load()),
  midiOutL       (o.midiOutL.load()),
  midiOutLplaying(o.midiOutLplaying.load()),
  midiOutLmute   (o.midiOutLmute.load()),
  midiOutLsolo   (o.midiOutLsolo.load())
#ifdef WITH_VST
 ,pluginIds      (o.pluginIds)
#endif
{
	buffer.alloc(o.buffer.countFrames(), G_MAX_IO_CHANS);
}


/* -------------------------------------------------------------------------- */


Channel::Channel(const patch::Channel& p, int bufferSize)
: type           (p.type),
  playStatus     (p.waveId == 0 ? ChannelStatus::EMPTY : ChannelStatus::OFF),
  recStatus      (ChannelStatus::OFF),
  columnId       (p.columnId),
  id             (p.id),
  previewMode    (PreviewMode::NONE),
  pan            (p.pan),
  volume         (p.volume),
  armed          (p.armed),
  name           (p.name),
  key            (p.key),
  mute           (p.mute),
  solo           (p.solo),
  volume_i       (1.0),
  volume_d       (0.0),
  hasActions     (p.hasActions),
  readActions    (p.readActions),
  midiIn         (p.midiIn),
  midiInKeyPress (p.midiInKeyPress),
  midiInKeyRel   (p.midiInKeyRel),
  midiInKill     (p.midiInKill),
  midiInArm      (p.midiInArm),
  midiInVolume   (p.midiInVolume),
  midiInMute     (p.midiInMute),
  midiInSolo     (p.midiInSolo),
  midiInFilter   (p.midiInFilter),
  midiOutL       (p.midiOutL),
  midiOutLplaying(p.midiOutLplaying),
  midiOutLmute   (p.midiOutLmute),
  midiOutLsolo   (p.midiOutLsolo)
#ifdef WITH_VST
 ,pluginIds      (p.pluginIds)
#endif
{
	buffer.alloc(bufferSize, G_MAX_IO_CHANS);
}


/* -------------------------------------------------------------------------- */


bool Channel::isPlaying() const
{
	return playStatus == ChannelStatus::PLAY || 
	       playStatus == ChannelStatus::ENDING;
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
	switch (playStatus) {
		case ChannelStatus::OFF:
			kernelMidi::sendMidiLightning(midiOutLplaying, midimap::stopped);
			break;
		case ChannelStatus::WAIT:
			kernelMidi::sendMidiLightning(midiOutLplaying, midimap::waiting);
			break;
		case ChannelStatus::ENDING:
			kernelMidi::sendMidiLightning(midiOutLplaying, midimap::stopping);
			break;
		case ChannelStatus::PLAY:
			if ((mixer::isChannelAudible(this) && !mute) || 
				!midimap::isDefined(midimap::playingInaudible))
				kernelMidi::sendMidiLightning(midiOutLplaying, midimap::playing);
			else
				kernelMidi::sendMidiLightning(midiOutLplaying, midimap::playingInaudible);
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
	if (v > 1.0f) v = 1.0f;
	else 
	if (v < 0.0f) v = 0.0f;
	pan = v;
}


float Channel::getPan() const
{
	return pan;
}


/* -------------------------------------------------------------------------- */


float Channel::calcPanning(int ch) const
{	
	float p = pan;
	if (p  == 0.5f) // center: nothing to do
		return 1.0;
	if (ch == 0)
		return 1.0 - p;
	else  // channel 1
		return p; 
}


/* -------------------------------------------------------------------------- */


void Channel::calcVolumeEnvelope()
{
	volume_i = volume_i + volume_d;
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

}} // giada::m::
