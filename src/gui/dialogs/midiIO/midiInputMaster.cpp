/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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
#include "../../../utils/gui.h"
#include "../../../core/conf.h"
#include "../../../core/const.h"
#include "../../elems/midiLearner.h"
#include "../../elems/basics/button.h"
#include "../../elems/basics/check.h"
#include "../../elems/basics/choice.h"
#include "midiInputMaster.h"


using namespace giada;
using namespace giada::m;


gdMidiInputMaster::gdMidiInputMaster()
	: gdMidiInputBase(0, 0, 300, 284, "MIDI Input Setup (global)")
{
	set_modal();

	Fl_Group* groupHeader = new Fl_Group(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN, w(), 20);
	groupHeader->begin();

		enable = new geCheck(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN, 120, G_GUI_UNIT, 
			"enable MIDI input");
		channel = new geChoice(enable->x()+enable->w()+44, G_GUI_OUTER_MARGIN, 120, G_GUI_UNIT);

	groupHeader->resizable(nullptr);
	groupHeader->end();

	Fl_Pack* pack = new Fl_Pack(G_GUI_OUTER_MARGIN, groupHeader->y()+groupHeader->h()+G_GUI_OUTER_MARGIN, 
		LEARNER_WIDTH, 212);
	pack->spacing(G_GUI_INNER_MARGIN);
	pack->begin();

		new geMidiLearner(0, 0, LEARNER_WIDTH, "rewind",           &cb_learn, &conf::midiInRewind, nullptr);
		new geMidiLearner(0, 0, LEARNER_WIDTH, "play/stop",        &cb_learn, &conf::midiInStartStop, nullptr);
		new geMidiLearner(0, 0, LEARNER_WIDTH, "action recording", &cb_learn, &conf::midiInActionRec, nullptr);
		new geMidiLearner(0, 0, LEARNER_WIDTH, "input recording",  &cb_learn, &conf::midiInInputRec, nullptr);
		new geMidiLearner(0, 0, LEARNER_WIDTH, "metronome",        &cb_learn, &conf::midiInMetronome, nullptr);
		new geMidiLearner(0, 0, LEARNER_WIDTH, "input volume",     &cb_learn, &conf::midiInVolumeIn, nullptr);
		new geMidiLearner(0, 0, LEARNER_WIDTH, "output volume",    &cb_learn, &conf::midiInVolumeOut, nullptr);
		new geMidiLearner(0, 0, LEARNER_WIDTH, "sequencer ×2",     &cb_learn, &conf::midiInBeatDouble, nullptr);
		new geMidiLearner(0, 0, LEARNER_WIDTH, "sequencer ÷2",     &cb_learn, &conf::midiInBeatHalf, nullptr);

	pack->end();

	ok = new geButton(w()-88, pack->y()+pack->h()+G_GUI_OUTER_MARGIN, 80, G_GUI_UNIT, "Close");

	end();

	ok->callback(cb_close, (void*)this);

	enable->value(conf::midiIn);
	enable->callback(cb_enable, (void*)this);

	channel->add("Channel (any)");
	channel->add("Channel 1");
	channel->add("Channel 2");
	channel->add("Channel 3");
	channel->add("Channel 4");
	channel->add("Channel 5");
	channel->add("Channel 6");
	channel->add("Channel 7");
	channel->add("Channel 8");
	channel->add("Channel 9");
	channel->add("Channel 10");
	channel->add("Channel 11");
	channel->add("Channel 12");
	channel->add("Channel 13");
	channel->add("Channel 14");
	channel->add("Channel 15");
	channel->add("Channel 16");
	channel->value(conf::midiInFilter -1 ? 0 : conf::midiInFilter + 1);
	channel->callback(cb_setChannel, (void*)this);

	u::gui::setFavicon(this);
	show();
}


/* -------------------------------------------------------------------------- */


void gdMidiInputMaster::cb_enable(Fl_Widget* w, void* p) { ((gdMidiInputMaster*)p)->cb_enable(); }
void gdMidiInputMaster::cb_setChannel(Fl_Widget* w, void* p) { ((gdMidiInputMaster*)p)->cb_setChannel(); }


/* -------------------------------------------------------------------------- */


void gdMidiInputMaster::cb_enable()
{
	conf::midiIn = enable->value();
	enable->value() ? channel->activate() : channel->deactivate();
}


/* -------------------------------------------------------------------------- */


void gdMidiInputMaster::cb_setChannel()
{
	conf::midiInFilter = channel->value() == 0 ? -1 : channel->value() - 1;
}

