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


#include <FL/Fl.H>
#include "core/model/model.h"
#include "core/channels/sampleChannel.h"
#include "glue/io.h"
#include "gui/elems/basics/button.h"
#include "midiLearnerChannel.h"


namespace giada {
namespace v 
{
geMidiLearnerChannel::geMidiLearnerChannel(int x, int y, int w, std::string l, int param, uint32_t value, ID channelId)
: geMidiLearnerBase(x, y, w, l, param, value),
  m_channelId      (channelId)
{
}


/* -------------------------------------------------------------------------- */


void geMidiLearnerChannel::refresh()
{
	m::model::onGet(m::model::channels, m_channelId, [&](const m::Channel& c)
	{
		switch (m_param) {
			case G_MIDI_IN_KEYPRESS     : update(c.midiInKeyPress); break;
			case G_MIDI_IN_KEYREL       : update(c.midiInKeyRel); break;
			case G_MIDI_IN_KILL         : update(c.midiInKill); break;
			case G_MIDI_IN_ARM          : update(c.midiInArm); break;
			case G_MIDI_IN_MUTE         : update(c.midiInVolume); break;
			case G_MIDI_IN_SOLO         : update(c.midiInMute); break;
			case G_MIDI_IN_VOLUME       : update(c.midiInSolo); break;
			case G_MIDI_IN_PITCH        : update(static_cast<const m::SampleChannel&>(c).midiInPitch); break;
			case G_MIDI_IN_READ_ACTIONS : update(static_cast<const m::SampleChannel&>(c).midiInReadActions); break;
			case G_MIDI_OUT_L_PLAYING   : update(static_cast<const m::SampleChannel&>(c).midiOutLplaying); break;
			case G_MIDI_OUT_L_MUTE      : update(static_cast<const m::SampleChannel&>(c).midiOutLmute); break;
			case G_MIDI_OUT_L_SOLO      : update(static_cast<const m::SampleChannel&>(c).midiOutLsolo); break;
		}
	});
}


/* -------------------------------------------------------------------------- */


void geMidiLearnerChannel::onLearn()
{
	if (m_button->value() == 1)
		c::io::startChannelMidiLearn(m_param, m_channelId);
	else
		c::io::stopMidiLearn();
}


void geMidiLearnerChannel::onReset()
{
	if (Fl::event_button() == FL_RIGHT_MOUSE)
		c::io::clearChannelMidiLearn(m_param, m_channelId);	
}
}} // giada::v::
