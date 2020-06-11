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


#ifndef G_CHANNEL_MIDI_RECEIVER_H
#define G_CHANNEL_MIDI_RECEIVER_H


#ifdef WITH_VST


#include <memory>


namespace giada {
namespace m
{
namespace mixer
{
struct Event;
}
struct MidiReceiverState;

/* MidiReceiver 
Takes live action gestures AND recorded actions and redirect them as MIDI events 
to plug-in soft synths. */

class MidiReceiver
{
public:

    MidiReceiver(ChannelState*);
    MidiReceiver(const patch::Channel&, ChannelState*);
    MidiReceiver(const MidiReceiver&, ChannelState* c=nullptr);

    void parse(const mixer::Event& e) const;
    void render(const std::vector<ID>& pluginIds) const;

    /* state
    Pointer to mutable MidiReceiverState state. */

    std::unique_ptr<MidiReceiverState> state;

private:

    /* parseMidi
    Takes a live message (e.g. from a MIDI keyboard), strips it and sends it
    to plug-ins. */

    void parseMidi(const MidiEvent& e) const;

	/* sendToPlugins
    Enqueues the MIDI event for plug-ins processing. This will be read later on 
    by the PluginHost. */

    void sendToPlugins(const MidiEvent& e, Frame localFrame) const;

    ChannelState* m_channelState;
};
}} // giada::m::


#endif // WITH_VST


#endif
