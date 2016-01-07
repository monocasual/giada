/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_midiIoTools
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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


#ifndef GE_LEARNER_H
#define GE_LEARNER_H


#include <FL/Fl.H>
#include <FL/Fl_Group.H>
#include "../../core/kernelMidi.h"
#include "../dialogs/gd_midiInput.h"


class gLearner : public Fl_Group
{
private:

	/* callback
	 * cb to pass to kernelMidi. Requires two parameters:
	 * uint32_t msg - MIDI message
	 * void   *data - extra data */

	kernelMidi::cb_midiLearn *callback;

	class gBox    *text;
	class gClick  *value;
	class gButton *button;

	static void cb_button(Fl_Widget *v, void *p);
	static void cb_value (Fl_Widget *v, void *p);
	inline void __cb_button();
	inline void __cb_value();

public:

	/* param
	 * pointer to ch->midiIn[value] */

	uint32_t *param;

	gLearner(int x, int y, int w, const char *l, kernelMidi::cb_midiLearn *cb, uint32_t *param);

	void updateValue();
};


/* -------------------------------------------------------------------------- */


/* cbData
 * struct we pass to kernelMidi as extra parameter. Local scope made
 * with unnamed namespace. Infos:
 * http://stackoverflow.com/questions/4422507/superiority-of-unnamed-namespace-over-static */

/* TODO - instead of the unnamed namespace, why don't we make the struct as a
(static) member of gLearner? */

namespace {
	struct cbData {
		gdMidiInput *window;
		gLearner    *learner;
	};
}


#endif
