/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
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
#include "../../../core/channel.h"
#include "../../../core/conf.h"
#include "../../../utils/log.h"
#include "../../elems/midiLearner.h"
#include "midiInputBase.h"


using std::string;
using namespace giada::m;


gdMidiInputBase::gdMidiInputBase(int x, int y, int w, int h, const char* title)
	: gdWindow(x, y, w, h, title)
{
}


/* -------------------------------------------------------------------------- */


gdMidiInputBase::~gdMidiInputBase()
{
	kernelMidi::stopMidiLearn();
}


/* -------------------------------------------------------------------------- */


void gdMidiInputBase::stopMidiLearn(geMidiLearner* learner)
{
	kernelMidi::stopMidiLearn();
	learner->updateValue();
}


/* -------------------------------------------------------------------------- */


void gdMidiInputBase::cb_learn(uint32_t* param, uint32_t msg, geMidiLearner* l)
{
	*param = msg;
	stopMidiLearn(l);
	gu_log("[gdMidiGrabber] MIDI learn done - message=0x%X\n", msg);
}


/* -------------------------------------------------------------------------- */


void gdMidiInputBase::cb_learn(uint32_t msg, void* d)
{

	geMidiLearner::cbData_t* data = (geMidiLearner::cbData_t*) d;
	geMidiLearner* learner = data->learner;
	Channel* channel = data->channel;
	uint32_t* param = learner->param;
	int midiChannel = (*param & 0x0F000000) >> 24; // Brutally extract channel

	/* No MIDI learning if we are learning a Channel (channel != nullptr) and 
	the selected MIDI channel is filtered OR if we are learning a global parameter
	(channel == nullptr) and the selected MIDI channel is filtered. */

	if ((channel != nullptr && !channel->isMidiInAllowed(midiChannel)) ||
	    (channel == nullptr && !conf::isMidiInAllowed(midiChannel)))
			return;

	gdMidiInputBase* window  = static_cast<gdMidiInputBase*>(data->window);
	window->cb_learn(param, msg, learner);
}


/* -------------------------------------------------------------------------- */


void gdMidiInputBase::cb_close(Fl_Widget* w, void* p)  { ((gdMidiInputBase*)p)->cb_close(); }


/* -------------------------------------------------------------------------- */


void gdMidiInputBase::cb_close()
{
	do_callback();
}
