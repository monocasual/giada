/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_midiGrabber
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013 Giovanni A. Zuliani | Monocasual
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
 * ------------------------------------------------------------------ */


#ifndef GD_MIDIGRABBER_H
#define GD_MIDIGRABBER_H


#include "ge_window.h"
#include "kernelMidi.h"
#include "utils.h"


class gLearn : public Fl_Group {

private:

	class Channel *ch;

	kernelMidi::cb_midiLearn *callback;

	class gBox    *text;
	class gButton *button;

	static void cb_button(Fl_Widget *v, void *p);
	inline void __cb_button();

public:

	gLearn(int x, int y, int w, const char *l, class Channel *ch, kernelMidi::cb_midiLearn *cb);
};


/* ------------------------------------------------------------------ */


class gdMidiGrabber : public gWindow {

private:

	class Channel *ch;

	gVector <gLearn *> items;


	/* cb_learnKeyPressRel
	 * callback attached to kernelMidi to learn key press and key release
	 * actions. */

	static void cb_learnKeyPressRel(uint32_t msg, void *data);
	inline void __cb_learnKeyPressRel(uint32_t msg);

	/** other callbacks here */

public:

	gdMidiGrabber(class Channel *ch);
	~gdMidiGrabber();
};

#endif
