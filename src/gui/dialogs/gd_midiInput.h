/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_midiInput
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


#ifndef GD_MIDI_INPUT_H
#define GD_MIDI_INPUT_H


#include "../../core/kernelMidi.h"
#include "../../utils/utils.h"
#include "../elems/ge_window.h"
#include "../elems/ge_mixed.h"


class gdMidiInput : public gWindow
{
protected:

	gClick *ok;

	void stopMidiLearn(class gLearner *l);

	/* cb_learn
	 * callback attached to kernelMidi to learn various actions. */

	static void cb_learn  (uint32_t msg, void *data);
	inline void __cb_learn(uint32_t *param, uint32_t msg, gLearner *l);

	static void cb_close  (Fl_Widget *w, void *p);
	inline void __cb_close();

public:

	gdMidiInput(int w, int h, const char *title);
	~gdMidiInput();
};


/* -------------------------------------------------------------------------- */


class gdMidiInputChannel : public gdMidiInput
{
private:

	class Channel *ch;

	gCheck *enable;


	//gVector <gLearner *> items; for future use, with vst parameters

	static void cb_enable  (Fl_Widget *w, void *p);
	inline void __cb_enable();

public:

	gdMidiInputChannel(class Channel *ch);
};


/* -------------------------------------------------------------------------- */


class gdMidiInputMaster : public gdMidiInput
{
public:

	gdMidiInputMaster();
};


#endif
