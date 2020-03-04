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
#include "glue/channel.h"
#include "midiChannelButton.h"


namespace giada {
namespace v
{
geMidiChannelButton::geMidiChannelButton(int x, int y, int w, int h, const c::channel::Data& d)
: geChannelButton(x, y, w, h, d)
{
}


/* -------------------------------------------------------------------------- */


void geMidiChannelButton::refresh()
{
	geChannelButton::refresh();

	refreshLabel();

	if (m_channel.a_isRecordingAction() && m_channel.a_isArmed())
		setActionRecordMode();
	
	redraw();
}


/* -------------------------------------------------------------------------- */


void geMidiChannelButton::refreshLabel()
{
    std::string l = m_channel.name.empty() ? "-- MIDI --" : m_channel.name; 

	if (m_channel.midi->a_isOutputEnabled())
		l += " (ch " + std::to_string(m_channel.midi->a_getFilter() + 1) + " out)";

	label(l.c_str());
}
}} // giada::v::
