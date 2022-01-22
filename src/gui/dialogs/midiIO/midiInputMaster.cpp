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

#include "midiInputMaster.h"
#include "core/conf.h"
#include "core/const.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/choice.h"
#include "gui/elems/basics/group.h"
#include "gui/elems/basics/scrollPack.h"
#include "gui/elems/midiIO/midiLearner.h"
#include "utils/gui.h"
#include <FL/Fl_Pack.H>

namespace giada::v
{
geMasterLearnerPack::geMasterLearnerPack(int x, int y)
: geMidiLearnerPack(x, y)
{
	setCallbacks(
	    [](int param) { c::io::master_startMidiLearn(param); },
	    [](int param) { c::io::master_clearMidiLearn(param); });
	addMidiLearner("rewind", G_MIDI_IN_REWIND);
	addMidiLearner("play/stop", G_MIDI_IN_START_STOP);
	addMidiLearner("action recording", G_MIDI_IN_ACTION_REC);
	addMidiLearner("input recording", G_MIDI_IN_INPUT_REC);
	addMidiLearner("metronome", G_MIDI_IN_METRONOME);
	addMidiLearner("input volume", G_MIDI_IN_VOLUME_IN);
	addMidiLearner("output volume", G_MIDI_IN_VOLUME_OUT);
	addMidiLearner("sequencer ×2", G_MIDI_IN_BEAT_DOUBLE);
	addMidiLearner("sequencer ÷2", G_MIDI_IN_BEAT_HALF);
}

/* -------------------------------------------------------------------------- */

void geMasterLearnerPack::update(const c::io::Master_InputData& d)
{
	learners[0]->update(d.rewind);
	learners[1]->update(d.startStop);
	learners[2]->update(d.actionRec);
	learners[3]->update(d.inputRec);
	learners[4]->update(d.metronome);
	learners[5]->update(d.volumeIn);
	learners[6]->update(d.volumeOut);
	learners[7]->update(d.beatDouble);
	learners[8]->update(d.beatHalf);
	setEnabled(d.enabled);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gdMidiInputMaster::gdMidiInputMaster(m::Conf::Data& c)
: gdMidiInputBase(c.midiInputX, c.midiInputY, 300, 284, "MIDI Input Setup (global)", c)
{
	end();

	geGroup* groupHeader = new geGroup(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN);
	m_enable             = new geCheck(0, 0, 120, G_GUI_UNIT, "Enable MIDI input");
	m_channel            = new geChoice(m_enable->x() + m_enable->w() + 44, 0, 120, G_GUI_UNIT);
	groupHeader->resizable(nullptr);
	groupHeader->add(m_enable);
	groupHeader->add(m_channel);

	m_learners = new geMasterLearnerPack(G_GUI_OUTER_MARGIN, groupHeader->y() + groupHeader->h() + G_GUI_OUTER_MARGIN);
	m_ok       = new geButton(w() - 88, m_learners->y() + m_learners->h() + G_GUI_OUTER_MARGIN, 80, G_GUI_UNIT, "Close");

	add(groupHeader);
	add(m_learners);
	add(m_ok);

	m_ok->callback(cb_close, (void*)this);
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
	m_channel->callback(cb_setChannel, (void*)this);

	u::gui::setFavicon(this);

	set_modal();
	rebuild();
	show();
}

/* -------------------------------------------------------------------------- */

void gdMidiInputMaster::rebuild()
{
	m_data = c::io::master_getInputData();

	m_enable->value(m_data.enabled);
	m_channel->value(m_data.filter - 1 ? 0 : m_data.filter + 1);
	m_learners->update(m_data);

	m_data.enabled ? m_channel->activate() : m_channel->deactivate();
}

/* -------------------------------------------------------------------------- */

void gdMidiInputMaster::cb_enable(Fl_Widget* /*w*/, void* p) { ((gdMidiInputMaster*)p)->cb_enable(); }
void gdMidiInputMaster::cb_setChannel(Fl_Widget* /*w*/, void* p) { ((gdMidiInputMaster*)p)->cb_setChannel(); }

/* -------------------------------------------------------------------------- */

void gdMidiInputMaster::cb_enable()
{
	c::io::master_enableMidiLearn(m_enable->value());
}

/* -------------------------------------------------------------------------- */

void gdMidiInputMaster::cb_setChannel()
{
	c::io::master_setMidiFilter(m_channel->value() == 0 ? -1 : m_channel->value() - 1);
}
} // namespace giada::v