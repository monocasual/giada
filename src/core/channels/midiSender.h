/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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


#ifndef G_CHANNEL_MIDI_SENDER_H
#define G_CHANNEL_MIDI_SENDER_H


namespace giada {
namespace m
{
namespace mixer
{
struct Event;
}
struct ChannelState;
struct MidiSenderState;
class MidiSender
{
public:

    MidiSender(ChannelState*);
    MidiSender(const patch::Channel&, ChannelState*);
    MidiSender(const MidiSender&, ChannelState* c=nullptr);

    void parse(const mixer::Event& e) const;

    /* state
    Pointer to mutable MidiSenderState state. */

    std::unique_ptr<MidiSenderState> state;

private:

    void send(MidiEvent e) const;

    ChannelState* m_channelState;
};
}} // giada::m::


#endif
