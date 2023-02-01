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

#include "gui/dialogs/midiIO/midiInputMaster.h"
#include "core/conf.h"
#include "core/const.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/choice.h"
#include "gui/elems/basics/group.h"
#include "gui/elems/basics/scrollPack.h"
#include "gui/elems/basics/textButton.h"
#include "gui/elems/midiIO/midiLearner.h"
#include "gui/ui.h"
#include "utils/gui.h"

extern giada::v::Ui g_ui;

namespace giada::v
{
geMasterLearnerPack::geMasterLearnerPack(int x, int y)
: geMidiLearnerPack(x, y)
{
	setCallbacks(
	    [](int param) { c::io::master_startMidiLearn(param); },
	    [](int param) { c::io::master_clearMidiLearn(param); });
	addMidiLearner(g_ui.getI18Text(LangMap::MIDIINPUT_MASTER_LEARN_REWIND), G_MIDI_IN_REWIND);
	addMidiLearner(g_ui.getI18Text(LangMap::MIDIINPUT_MASTER_LEARN_PLAYSTOP), G_MIDI_IN_START_STOP);
	addMidiLearner(g_ui.getI18Text(LangMap::MIDIINPUT_MASTER_LEARN_ACTIONREC), G_MIDI_IN_ACTION_REC);
	addMidiLearner(g_ui.getI18Text(LangMap::MIDIINPUT_MASTER_LEARN_INPUTREC), G_MIDI_IN_INPUT_REC);
	addMidiLearner(g_ui.getI18Text(LangMap::MIDIINPUT_MASTER_LEARN_METRONOME), G_MIDI_IN_METRONOME);
	addMidiLearner(g_ui.getI18Text(LangMap::MIDIINPUT_MASTER_LEARN_INVOLUME), G_MIDI_IN_VOLUME_IN);
	addMidiLearner(g_ui.getI18Text(LangMap::MIDIINPUT_MASTER_LEARN_OUTVOLUME), G_MIDI_IN_VOLUME_OUT);
	addMidiLearner(g_ui.getI18Text(LangMap::MIDIINPUT_MASTER_LEARN_SEQDOUBLE), G_MIDI_IN_BEAT_DOUBLE);
	addMidiLearner(g_ui.getI18Text(LangMap::MIDIINPUT_MASTER_LEARN_SEQHALF), G_MIDI_IN_BEAT_HALF);
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

gdMidiInputMaster::gdMidiInputMaster(m::Conf& c)
: gdMidiInputBase(g_ui.getI18Text(LangMap::MIDIINPUT_MASTER_TITLE), c)
{
	end();

	geGroup* groupHeader = new geGroup(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN);
	m_enable             = new geCheck(0, 0, 120, G_GUI_UNIT, g_ui.getI18Text(LangMap::MIDIINPUT_MASTER_ENABLE));
	m_channel            = new geChoice(m_enable->x() + m_enable->w() + 44, 0, 120, G_GUI_UNIT);
	groupHeader->resizable(nullptr);
	groupHeader->add(m_enable);
	groupHeader->add(m_channel);

	m_learners = new geMasterLearnerPack(G_GUI_OUTER_MARGIN, groupHeader->y() + groupHeader->h() + G_GUI_OUTER_MARGIN);
	m_ok       = new geTextButton(w() - 88, m_learners->y() + m_learners->h() + G_GUI_OUTER_MARGIN, 80, G_GUI_UNIT, g_ui.getI18Text(LangMap::COMMON_CLOSE));

	add(groupHeader);
	add(m_learners);
	add(m_ok);

	m_ok->onClick = [this]() { do_callback(); };
	m_enable->callback(cb_enable, (void*)this);

	m_channel->addItem("Channel (any)");
	m_channel->addItem("Channel 1");
	m_channel->addItem("Channel 2");
	m_channel->addItem("Channel 3");
	m_channel->addItem("Channel 4");
	m_channel->addItem("Channel 5");
	m_channel->addItem("Channel 6");
	m_channel->addItem("Channel 7");
	m_channel->addItem("Channel 8");
	m_channel->addItem("Channel 9");
	m_channel->addItem("Channel 10");
	m_channel->addItem("Channel 11");
	m_channel->addItem("Channel 12");
	m_channel->addItem("Channel 13");
	m_channel->addItem("Channel 14");
	m_channel->addItem("Channel 15");
	m_channel->addItem("Channel 16");
	m_channel->onChange = [](ID id) {
		c::io::master_setMidiFilter(id == 0 ? -1 : id - 1);
	};

	set_modal();
	rebuild();
	show();
}

/* -------------------------------------------------------------------------- */

void gdMidiInputMaster::rebuild()
{
	m_data = c::io::master_getInputData();

	m_enable->value(m_data.enabled);
	m_channel->showItem(m_data.filter - 1 ? 0 : m_data.filter + 1);
	m_learners->update(m_data);

	m_data.enabled ? m_channel->activate() : m_channel->deactivate();
}

/* -------------------------------------------------------------------------- */

void gdMidiInputMaster::cb_enable(Fl_Widget* /*w*/, void* p) { ((gdMidiInputMaster*)p)->cb_enable(); }

/* -------------------------------------------------------------------------- */

void gdMidiInputMaster::cb_enable()
{
	c::io::master_enableMidiLearn(m_enable->value());
}
} // namespace giada::v