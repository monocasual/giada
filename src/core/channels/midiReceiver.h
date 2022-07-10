/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "core/channels/channelShared.h"
#include "core/sequencer.h"

namespace giada::m
{
class PluginHost;
class Plugin;
class MidiReceiver final
{
public:
	void react(ChannelShared::MidiQueue&, const EventDispatcher::Event&) const;
	void advance(ID channelId, ChannelShared::MidiQueue&, const Sequencer::Event&) const;
	void render(ChannelShared&, const std::vector<Plugin*>&, PluginHost&) const;

private:
	void sendToPlugins(ChannelShared::MidiQueue&, const MidiEvent&, Frame localFrame) const;
	void parseMidi(ChannelShared::MidiQueue&, const MidiEvent&) const;
};
} // namespace giada::m

#endif