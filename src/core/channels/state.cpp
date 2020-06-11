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


#include "core/patch.h"
#include "state.h"


namespace giada {
namespace m 
{
MidiLearnerState::MidiLearnerState()
: enabled      (true)
, filter       (0)
, keyPress     (0x0)
, keyRelease   (0x0)
, kill         (0x0)
, arm          (0x0)
, volume       (0x0)
, mute         (0x0)
, solo         (0x0)
, readActions  (0x0)
, pitch        (0x0)
{
}


MidiLearnerState::MidiLearnerState(const patch::Channel& p)
: enabled      (p.midiIn)
, filter       (p.midiInFilter)
, keyPress     (p.midiInKeyPress)
, keyRelease   (p.midiInKeyRel)
, kill         (p.midiInKill)
, arm          (p.midiInArm)
, volume       (p.midiInVolume)
, mute         (p.midiInMute)
, solo         (p.midiInSolo)
, readActions  (p.midiInReadActions)
, pitch        (p.midiInPitch)
{
}


MidiLearnerState::MidiLearnerState(const MidiLearnerState& o)
: enabled      (o.enabled.load())
, filter       (o.filter.load())
, keyPress     (o.keyPress.load())
, keyRelease   (o.keyRelease.load())
, kill         (o.kill.load())
, arm          (o.arm.load())
, volume       (o.volume.load())
, mute         (o.mute.load())
, solo         (o.solo.load())
, readActions  (o.readActions.load())
, pitch        (o.pitch.load())
{
}


/* -------------------------------------------------------------------------- */


bool MidiLearnerState::isAllowed(int c) const
{
    int filter_   = filter.load();
    bool enabled_ = enabled.load();

	return enabled_ && (filter_ == -1 || filter_ == c);
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


MidiLighterState::MidiLighterState()
: enabled(false)
, playing(0x0)
, mute   (0x0)
, solo   (0x0)
{
}


MidiLighterState::MidiLighterState(const patch::Channel& p)
: enabled(p.midiOutL)
, playing(p.midiOutLplaying)
, mute   (p.midiOutLmute)
, solo   (p.midiOutLsolo)
{
}


MidiLighterState::MidiLighterState(const MidiLighterState& o)
: enabled(o.enabled.load())
, playing(o.playing.load())
, mute   (o.mute.load())
, solo   (o.solo.load())
{
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


MidiSenderState::MidiSenderState()
: enabled(true)
, filter (0)
{
}


MidiSenderState::MidiSenderState(const patch::Channel& p)
: enabled(p.midiOut)
, filter (p.midiOutChan)
{
}


MidiSenderState::MidiSenderState(const MidiSenderState& o)
: enabled(o.enabled.load())
, filter (o.filter.load())
{
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


#ifdef WITH_VST

MidiReceiverState::MidiReceiverState()
{
    midiBuffer.ensureSize(G_DEFAULT_VST_MIDIBUFFER_SIZE);
}

#endif


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


SamplePlayerState::SamplePlayerState()
: tracker      (0)
, pitch        (G_DEFAULT_PITCH)
, mode         (SamplePlayerMode::SINGLE_BASIC)
, velocityAsVol(false)
, rewinding    (false)
, quantizing   (false)
, offset       (0)
{
}


SamplePlayerState::SamplePlayerState(const SamplePlayerState& o)
: tracker      (o.tracker.load())
, pitch        (o.pitch.load())
, mode         (o.mode.load())
, shift        (o.shift.load())
, begin        (o.begin.load())
, end          (o.end.load())
, velocityAsVol(o.velocityAsVol.load())
, rewinding    (o.rewinding)
, quantizing   (o.quantizing)
, offset       (o.offset)
, quantizer    (o.quantizer)
{
}


SamplePlayerState::SamplePlayerState(const patch::Channel& p)
: tracker      (0)
, pitch        (p.pitch)
, mode         (p.mode)
, shift        (p.shift)
, begin        (p.begin)
, end          (p.end)
, velocityAsVol(p.midiInVeloAsVol)
, rewinding    (false)
, quantizing   (false)
, offset       (0)
{
}


/* -------------------------------------------------------------------------- */


bool SamplePlayerState::isAnyLoopMode() const
{
    SamplePlayerMode m = mode.load();

	return m == SamplePlayerMode::LOOP_BASIC  || 
	       m == SamplePlayerMode::LOOP_ONCE   || 
	       m == SamplePlayerMode::LOOP_REPEAT || 
	       m == SamplePlayerMode::LOOP_ONCE_BAR;
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


AudioReceiverState::AudioReceiverState()
: inputMonitor(false)
{
}


AudioReceiverState::AudioReceiverState(const patch::Channel& p)
: inputMonitor(p.inputMonitor)
{
}


AudioReceiverState::AudioReceiverState(const AudioReceiverState& o)
: inputMonitor(o.inputMonitor.load())
{
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


ChannelState::ChannelState(ID id, Frame bufferSize)
: id         (id)
, playStatus (ChannelStatus::OFF)
, recStatus  (ChannelStatus::OFF)
, volume     (G_DEFAULT_VOL)
, pan        (G_DEFAULT_PAN)
, mute       (false)
, solo       (false)
, armed      (false)
, key        (0)
, readActions(true)
, buffer     (bufferSize, G_MAX_IO_CHANS)
, hasActions (false)
, height     (G_GUI_UNIT)
, volume_i   (1.0f)
{
}
    

ChannelState::ChannelState(const ChannelState& o)
: id         (o.id)
, playStatus (o.playStatus.load())
, recStatus  (o.recStatus.load())
, volume     (o.volume.load())
, pan        (o.pan.load())
, mute       (o.mute.load())
, solo       (o.solo.load())
, armed      (o.armed.load())
, key        (o.key.load())
, readActions(o.readActions.load())
, buffer     (o.buffer)
, hasActions (o.hasActions)
, name       (o.name)
, height     (o.height)
, volume_i   (o.volume_i)
{
}


ChannelState::ChannelState(const patch::Channel& p, Frame bufferSize)
: id         (p.id)
, playStatus (ChannelStatus::OFF)
, recStatus  (ChannelStatus::OFF)
, volume     (p.volume)
, pan        (p.pan)
, mute       (p.mute)
, solo       (p.solo)
, armed      (p.armed)
, key        (p.key)
, readActions(p.readActions)
, buffer     (bufferSize, G_MAX_IO_CHANS)
, hasActions (p.hasActions)
, name       (p.name)
, height     (p.height)
, volume_i   (1.0f)
{
}
    

/* -------------------------------------------------------------------------- */


bool ChannelState::isPlaying() const
{
    ChannelStatus s = playStatus.load();
	return s == ChannelStatus::PLAY || s == ChannelStatus::ENDING;
}
}} // giada::m::
