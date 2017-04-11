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


#include "ge_mixed.h"
#include "basics/boxtypes.h"
#include "midiLearner.h"


using namespace giada;


geMidiLearner::geMidiLearner(int X, int Y, int W, const char *l,
  kernelMidi::cb_midiLearn *cb, uint32_t *param)
	: Fl_Group(X, Y, W, 20),
		callback(cb),
		param   (param)
{
	begin();
	text   = new gBox(x(), y(), 156, 20, l);
	value  = new geButton(text->x()+text->w()+4, y(), 80, 20);
	button = new gButton(value->x()+value->w()+4, y(), 40, 20, "learn");
	end();

	text->box(G_CUSTOM_BORDER_BOX);
	text->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

	value->box(G_CUSTOM_BORDER_BOX);
	value->callback(cb_value, (void*)this);
	value->when(FL_WHEN_RELEASE);
	updateValue();

	button->type(FL_TOGGLE_BUTTON);
	button->callback(cb_button, (void*)this);
}


/* -------------------------------------------------------------------------- */


void geMidiLearner::updateValue()
{
	char buf[16];
	if (*param != 0x0)
		snprintf(buf, 9, "0x%X", *param);
	else
		snprintf(buf, 16, "(not set)");
	value->copy_label(buf);
	button->value(0);
}


/* -------------------------------------------------------------------------- */


void geMidiLearner::cb_button(Fl_Widget *v, void *p) { ((geMidiLearner*)p)->__cb_button(); }
void geMidiLearner::cb_value(Fl_Widget *v, void *p) { ((geMidiLearner*)p)->__cb_value(); }


/* -------------------------------------------------------------------------- */


void geMidiLearner::__cb_value()
{
	if (Fl::event_button() == FL_RIGHT_MOUSE) {
		*param = 0x0;
		updateValue();
	}
	/// TODO - elif (LEFT_MOUSE) : insert values by hand
}


/* -------------------------------------------------------------------------- */


void geMidiLearner::__cb_button()
{
	if (button->value() == 1) {
		cbData.window  = (gdMidiInput*) parent();  // parent = gdMidiInput
		cbData.learner = this;
		kernelMidi::startMidiLearn(callback, (void*)&cbData);
	}
	else
		kernelMidi::stopMidiLearn();
}
