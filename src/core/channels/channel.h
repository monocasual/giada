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


#ifndef G_CHANNEL_H
#define G_CHANNEL_H


#include <optional>
#include "core/const.h"
#include "core/mixer.h"
#include "core/channels/state.h"
#include "core/channels/samplePlayer.h"
#include "core/channels/audioReceiver.h"
#include "core/channels/midiReceiver.h"
#include "core/channels/midiLearner.h"
#include "core/channels/midiSender.h"
#include "core/channels/midiLighter.h"
#include "core/channels/sampleActionRecorder.h"
#include "core/channels/midiActionRecorder.h"


namespace giada {
namespace m
{
class Channel final
{
public:

    Channel(ChannelType t, ID id, ID columnId, Frame bufferSize);
    Channel(const Channel&);
    Channel(const patch::Channel& p, Frame bufferSize);
    Channel(Channel&&)                 = default;
    Channel& operator=(const Channel&) = default;
    Channel& operator=(Channel&&)      = default;
    ~Channel()                         = default;

    /* parse
    Parses live events. */

    void parse(const mixer::EventBuffer& e, bool audible) const;

    /* advance
    Processes static events (e.g. actions) in the current block. */

    void advance(Frame bufferSize) const;

    /* render
    Renders audio data to I/O buffers. */
     
    void render(AudioBuffer* out, AudioBuffer* in, bool audible) const;

    bool isInternal() const;
    bool isMuted() const;
    bool canInputRec() const;
    ID getColumnId() const;
    ChannelType getType() const;
    
    ID id;

#ifdef WITH_VST
    std::vector<ID> pluginIds;
#endif

    /* state
    Pointer to mutable Channel state. */

    std::unique_ptr<ChannelState> state;

    /* midiLearner
    Holds MIDI learnt commands. */

    MidiLearner midiLearner;

    /* midiLighter
    Emits MIDI lightning messages. */

    MidiLighter midiLighter;

    /* (optional) samplePlayer
    For sample rendering. Sample Channel only. */

    std::optional<SamplePlayer> samplePlayer;

    /* (optional) audioReceiver
    For input audio. Sample Channel only. */

    std::optional<AudioReceiver> audioReceiver;

    /* (optional) midiReceiver
    Receives MIDI messages and events. MIDI Channel only. */

    std::optional<MidiReceiver> midiReceiver;

    /* (optional) midiSender
    Sends MIDI messages to the outside world. MIDI Channel only. */

    std::optional<MidiSender> midiSender;

    /* (optional) sampleActionRecorder, midiActionRecorder
    Records Actions from the outside world. */
    
    std::optional<SampleActionRecorder> sampleActionRecorder;
    std::optional<MidiActionRecorder>   midiActionRecorder;

private:

    void parse(const mixer::Event& e) const;

    void renderMasterOut(AudioBuffer& out) const;
    void renderMasterIn(AudioBuffer& in) const;
    void renderChannel(AudioBuffer& out, AudioBuffer& in, bool audible) const;

    AudioBuffer::Pan calcPanning() const;

    ChannelType m_type;
    ID m_columnId;
};
}} // giada::m::


#endif
