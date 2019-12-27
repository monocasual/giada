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


#include "core/model/model.h"
#include "core/channels/sampleChannel.h"
#include "utils/gui.h"
#include "gui/elems/midiIO/midiLearnerChannel.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/check.h"
#include "midiOutputSampleCh.h"


namespace giada {
namespace v 
{
gdMidiOutputSampleCh::gdMidiOutputSampleCh(ID channelId)
: gdMidiOutputBase(300, 140, channelId)
{
	m::model::ChannelsLock l(m::model::channels);
	m::Channel& c = m::model::get(m::model::channels, m_channelId);
	
	setTitle(c.id);

	m_enableLightning = new geCheck(8, 8, 120, 20, "Enable MIDI lightning output");
	m_learners.push_back(new geMidiLearnerChannel(8, m_enableLightning->y()+m_enableLightning->h()+8, w()-16, "playing", 
		G_MIDI_OUT_L_PLAYING, c.midiOutLplaying, m_channelId));
	m_learners.push_back(new geMidiLearnerChannel(8, m_enableLightning->y()+m_enableLightning->h()+32, w()-16, "mute",   
		G_MIDI_OUT_L_MUTE, c.midiOutLmute, m_channelId));
	m_learners.push_back(new geMidiLearnerChannel(8, m_enableLightning->y()+m_enableLightning->h()+56, w()-16, "solo",   
		G_MIDI_OUT_L_SOLO, c.midiOutLsolo, m_channelId));

	m_close = new geButton(w()-88, m_enableLightning->y()+m_enableLightning->h()+84, 80, 20, "Close");
	m_close->callback(cb_close, (void*)this);

	m_enableLightning->value(c.midiOutL);
	m_enableLightning->callback(cb_enableLightning, (void*)this);
	
	for (geMidiLearnerBase* l : m_learners)
		c.midiOutL ? l->activate() : l->deactivate();

	set_modal();
	u::gui::setFavicon(this);
	show();
}
}} // giada::v::
