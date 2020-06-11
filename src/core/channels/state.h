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


#ifndef G_CHANNEL_STATE_H
#define G_CHANNEL_STATE_H


#include <string>
#include <atomic>
#include "core/const.h"
#include "core/types.h"
#include "core/quantizer.h"
#include "core/audioBuffer.h"
#ifdef WITH_VST
#include "deps/juce-config.h"
#endif


namespace giada {
namespace m {
namespace patch
{
struct Channel;
}
struct MidiLearnerState
{
    MidiLearnerState();
    MidiLearnerState(const patch::Channel& p);
    MidiLearnerState(const MidiLearnerState& o);

    /* isAllowed
    Tells whether the current MIDI channel 'channel' is enabled to receive MIDI
    data. */

    bool isAllowed(int channel) const;

    /* enabled
    Tells whether MIDI learning is enabled for the current channel. */
    
	std::atomic<bool> enabled;

    /* filter
    Which MIDI channel should be filtered out when receiving MIDI messages. 
    If -1 means 'all'. */
    
    std::atomic<int> filter;

    /* MIDI learning fields. */

	std::atomic<uint32_t> keyPress;
	std::atomic<uint32_t> keyRelease;
	std::atomic<uint32_t> kill;
	std::atomic<uint32_t> arm;
	std::atomic<uint32_t> volume;
	std::atomic<uint32_t> mute;
	std::atomic<uint32_t> solo;
	std::atomic<uint32_t> readActions; // Sample Channels only
	std::atomic<uint32_t> pitch;       // Sample Channels only
};


/* -------------------------------------------------------------------------- */


struct MidiLighterState
{
    MidiLighterState();
    MidiLighterState(const patch::Channel& p);
    MidiLighterState(const MidiLighterState& o);

    /* enabled
    Tells whether MIDI ligthing is enabled or not. */
    
	std::atomic<bool> enabled;

    /* MIDI learning fields for MIDI ligthing. */

	std::atomic<uint32_t> playing;
	std::atomic<uint32_t> mute;
	std::atomic<uint32_t> solo;
};


/* -------------------------------------------------------------------------- */


struct MidiSenderState
{
    MidiSenderState();
    MidiSenderState(const patch::Channel& p);
    MidiSenderState(const MidiSenderState& o);

    /* enabled
    Tells whether MIDI output is enabled or not. */
    
	std::atomic<bool> enabled;

    /* filter
    Which MIDI channel data should be sent to. */
    
    std::atomic<int> filter;
};


/* -------------------------------------------------------------------------- */


#ifdef WITH_VST

struct MidiReceiverState
{
    MidiReceiverState();

	/* midiBuffer 
	Contains MIDI events to be sent to plug-ins. */

	juce::MidiBuffer midiBuffer;
};

#endif


/* -------------------------------------------------------------------------- */


struct SamplePlayerState
{
    SamplePlayerState();
    SamplePlayerState(const patch::Channel& p);
    SamplePlayerState(const SamplePlayerState& o);

    bool isAnyLoopMode() const;

    std::atomic<Frame>            tracker;
    std::atomic<float>            pitch;
    std::atomic<SamplePlayerMode> mode;
    std::atomic<Frame>            shift;
    std::atomic<Frame>            begin;
    std::atomic<Frame>            end;

    /* velocityAsVol
    Velocity drives volume. */

	std::atomic<bool> velocityAsVol;

    bool      rewinding;
    bool      quantizing;
    Frame     offset;
    Quantizer quantizer;
};


/* -------------------------------------------------------------------------- */


struct AudioReceiverState
{
    AudioReceiverState();
    AudioReceiverState(const patch::Channel& p);
    AudioReceiverState(const AudioReceiverState& o);

    std::atomic<bool> inputMonitor;
};


/* -------------------------------------------------------------------------- */


struct ChannelState
{
    ChannelState(ID id, Frame bufferSize);
    ChannelState(const patch::Channel& p, Frame bufferSize);
    ChannelState(const ChannelState& o);

    bool isPlaying() const;

    ID id;

    std::atomic<ChannelStatus> playStatus;
	std::atomic<ChannelStatus> recStatus;
    std::atomic<float>         volume;
    std::atomic<float>         pan;
    std::atomic<bool>          mute;
    std::atomic<bool>          solo;
    std::atomic<bool>          armed;
    std::atomic<int>           key;
	std::atomic<bool>          readActions;
	
	/* buffer (internal)
	Working buffer for internal processing. */

    AudioBuffer buffer;

    bool        hasActions;
    std::string name;
    Pixel       height;

	/* volume_i (internal)
	Internal volume used for volume automation and velocity-drives-volume mode
    on Sample Channels. */

    float volume_i;
};
}} // giada::m::


#endif
