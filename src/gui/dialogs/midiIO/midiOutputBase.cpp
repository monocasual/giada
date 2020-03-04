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


#include "glue/io.h"
#include "gui/elems/midiIO/midiLearner.h"
#include "gui/elems/basics/check.h"
#include "midiOutputBase.h"


namespace giada {
namespace v 
{
geLightningLearnerPack::geLightningLearnerPack(int x, int y, ID channelId)
: geMidiLearnerPack(x, y)
{
	setCallbacks(
		[channelId] (int param) { c::io::channel_startMidiLearn(param, channelId); },
		[channelId] (int param) { c::io::channel_clearMidiLearn(param, channelId); }
	);
	addMidiLearner("playing", G_MIDI_OUT_L_PLAYING);
	addMidiLearner("mute",    G_MIDI_OUT_L_MUTE);
	addMidiLearner("solo",    G_MIDI_OUT_L_SOLO);	
}


/* -------------------------------------------------------------------------- */


void geLightningLearnerPack::update(const c::io::Channel_OutputData& d)
{
	learners[0]->update(d.lightningPlaying);
	learners[1]->update(d.lightningMute);
	learners[2]->update(d.lightningSolo);
	setEnabled(d.lightningEnabled);
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


gdMidiOutputBase::gdMidiOutputBase(int w, int h, ID channelId)
: gdWindow   (w, h, "Midi Output Setup")
, m_channelId(channelId)
{
}


/* -------------------------------------------------------------------------- */


gdMidiOutputBase::~gdMidiOutputBase()
{
	c::io::stopMidiLearn();
}


/* -------------------------------------------------------------------------- */


void gdMidiOutputBase::cb_close(Fl_Widget* w, void* p)           { ((gdMidiOutputBase*)p)->cb_close(); }
void gdMidiOutputBase::cb_enableLightning(Fl_Widget *w, void *p) { ((gdMidiOutputBase*)p)->cb_enableLightning(); }


/* -------------------------------------------------------------------------- */


void gdMidiOutputBase::cb_close()
{
	do_callback();
}


/* -------------------------------------------------------------------------- */


void gdMidiOutputBase::cb_enableLightning()
{
	c::io::channel_enableMidiLightning(m_channelId, m_enableLightning->value());
}


/* -------------------------------------------------------------------------- */


void gdMidiOutputBase::setTitle(ID channelId)
{
	std::string tmp = "MIDI Output Setup (channel " + std::to_string(channelId) + ")"; 
	copy_label(tmp.c_str());
}
}} // giada::v::
