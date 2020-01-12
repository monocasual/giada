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


#include "utils/string.h"
#include "core/channels/midiChannel.h"
#include "core/model/model.h"
#include "core/recManager.h"
#include "midiChannelButton.h"


namespace giada {
namespace v
{
geMidiChannelButton::geMidiChannelButton(int x, int y, int w, int h, ID channelId)
: geChannelButton(x, y, w, h, channelId)
{
    std::string l; 
	m::model::onGet(m::model::channels, m_channelId, [&](m::Channel& c)
	{
		const m::MidiChannel& mc = static_cast<m::MidiChannel&>(c);
		if (mc.name.empty())
			l = "-- MIDI --";
		else
			l = mc.name.c_str();

		if (mc.midiOut) 
			l += " (ch " + u::string::iToString(mc.midiOutChan + 1) + " out)";
	});

    label(l.c_str());
}


/* -------------------------------------------------------------------------- */


void geMidiChannelButton::refresh()
{
	geChannelButton::refresh();

	m::model::onGet(m::model::channels, m_channelId, [&](m::Channel& c)
	{
		if (m::recManager::isRecordingAction() && c.armed)
			setActionRecordMode();
	});
	
	redraw();
}
}} // giada::v::
