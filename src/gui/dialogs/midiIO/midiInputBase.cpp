/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * midiInputBase
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#include "../../../core/kernelMidi.h"
#include "../../../utils/log.h"
#include "../../elems/midiLearner.h"
#include "midiInputBase.h"


using std::string;
using namespace giada;


gdMidiInputBase::gdMidiInputBase(int x, int y, int w, int h, const char *title)
	: gWindow(x, y, w, h, title)
{
}


/* -------------------------------------------------------------------------- */


gdMidiInputBase::~gdMidiInputBase()
{
	kernelMidi::stopMidiLearn();
}


/* -------------------------------------------------------------------------- */


void gdMidiInputBase::stopMidiLearn(geMidiLearner *learner)
{
	kernelMidi::stopMidiLearn();
	learner->updateValue();
}


/* -------------------------------------------------------------------------- */


void gdMidiInputBase::__cb_learn(uint32_t *param, uint32_t msg, geMidiLearner *l)
{
	*param = msg;
	stopMidiLearn(l);
	gu_log("[gdMidiGrabber] MIDI learn done - message=0x%X\n", msg);
}


/* -------------------------------------------------------------------------- */


void gdMidiInputBase::cb_learn(uint32_t msg, void *d)
{
	geMidiLearner::cbData_t *data = (geMidiLearner::cbData_t *) d;
	gdMidiInputBase *window  = (gdMidiInputBase*) data->window;
	geMidiLearner   *learner = data->learner;
	uint32_t        *param   = learner->param;
	window->__cb_learn(param, msg, learner);
}


/* -------------------------------------------------------------------------- */


void gdMidiInputBase::cb_close(Fl_Widget *w, void *p)  { ((gdMidiInputBase*)p)->__cb_close(); }


/* -------------------------------------------------------------------------- */


void gdMidiInputBase::__cb_close()
{
	do_callback();
}
