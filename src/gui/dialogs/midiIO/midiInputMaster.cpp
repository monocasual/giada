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


#include <FL/Fl_Pack.H>
#include "utils/gui.h"
#include "core/conf.h"
#include "core/const.h"
#include "gui/elems/midiLearner.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/choice.h"
#include "midiInputMaster.h"


namespace giada {
namespace v 
{
gdMidiInputMaster::gdMidiInputMaster()
: gdMidiInputBase(0, 0, 300, 284, "MIDI Input Setup (global)")
{
	set_modal();

	Fl_Group* groupHeader = new Fl_Group(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN, w(), 20);
	groupHeader->begin();

		m_enable = new geCheck(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN, 120, G_GUI_UNIT, 
			"m_enable MIDI input");
		m_channel = new geChoice(m_enable->x()+m_enable->w()+44, G_GUI_OUTER_MARGIN, 120, G_GUI_UNIT);

	groupHeader->resizable(nullptr);
	groupHeader->end();

	Fl_Pack* pack = new Fl_Pack(G_GUI_OUTER_MARGIN, groupHeader->y()+groupHeader->h()+G_GUI_OUTER_MARGIN, 
		LEARNER_WIDTH, 212);
	pack->spacing(G_GUI_INNER_MARGIN);
	pack->begin();
		m_learners.push_back(new geMidiLearner(0, 0, LEARNER_WIDTH, "rewind",           m::conf::midiInRewind,     0));
		m_learners.push_back(new geMidiLearner(0, 0, LEARNER_WIDTH, "play/stop",        m::conf::midiInStartStop,  0));
		m_learners.push_back(new geMidiLearner(0, 0, LEARNER_WIDTH, "action recording", m::conf::midiInActionRec,  0));
		m_learners.push_back(new geMidiLearner(0, 0, LEARNER_WIDTH, "input recording",  m::conf::midiInInputRec,   0));
		m_learners.push_back(new geMidiLearner(0, 0, LEARNER_WIDTH, "metronome",        m::conf::midiInMetronome,  0));
		m_learners.push_back(new geMidiLearner(0, 0, LEARNER_WIDTH, "input volume",     m::conf::midiInVolumeIn,   0));
		m_learners.push_back(new geMidiLearner(0, 0, LEARNER_WIDTH, "output volume",    m::conf::midiInVolumeOut,  0));
		m_learners.push_back(new geMidiLearner(0, 0, LEARNER_WIDTH, "sequencer ×2",     m::conf::midiInBeatDouble, 0));
		m_learners.push_back(new geMidiLearner(0, 0, LEARNER_WIDTH, "sequencer ÷2",     m::conf::midiInBeatHalf,   0));
	pack->end();

	m_ok = new geButton(w()-88, pack->y()+pack->h()+G_GUI_OUTER_MARGIN, 80, G_GUI_UNIT, "Close");

	end();

	m_ok->callback(cb_close, (void*)this);

	m_enable->value(m::conf::midiIn);
	m_enable->callback(cb_enable, (void*)this);

	m_channel->add("Channel (any)");
	m_channel->add("Channel 1");
	m_channel->add("Channel 2");
	m_channel->add("Channel 3");
	m_channel->add("Channel 4");
	m_channel->add("Channel 5");
	m_channel->add("Channel 6");
	m_channel->add("Channel 7");
	m_channel->add("Channel 8");
	m_channel->add("Channel 9");
	m_channel->add("Channel 10");
	m_channel->add("Channel 11");
	m_channel->add("Channel 12");
	m_channel->add("Channel 13");
	m_channel->add("Channel 14");
	m_channel->add("Channel 15");
	m_channel->add("Channel 16");
	m_channel->value(m::conf::midiInFilter -1 ? 0 : m::conf::midiInFilter + 1);
	m_channel->callback(cb_setChannel, (void*)this);

	u::gui::setFavicon(this);
	show();
}


/* -------------------------------------------------------------------------- */


void gdMidiInputMaster::cb_enable(Fl_Widget* w, void* p) { ((gdMidiInputMaster*)p)->cb_enable(); }
void gdMidiInputMaster::cb_setChannel(Fl_Widget* w, void* p) { ((gdMidiInputMaster*)p)->cb_setChannel(); }


/* -------------------------------------------------------------------------- */


void gdMidiInputMaster::cb_enable()
{
	m::conf::midiIn = m_enable->value();
	m_enable->value() ? m_channel->activate() : m_channel->deactivate();
}


/* -------------------------------------------------------------------------- */


void gdMidiInputMaster::cb_setChannel()
{
	m::conf::midiInFilter = m_channel->value() == 0 ? -1 : m_channel->value() - 1;
}

}} // giada::v::
