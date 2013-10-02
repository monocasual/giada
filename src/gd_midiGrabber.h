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


class gdMidiGrabber : public gWindow {

private:

	class gBox    *text;
	class Channel *ch;

	/* cb_midiLearn
	 * callback called by kernelMidi when midi learn is done. */

	static void cb_midiLearn(void *data);
	inline void __cb_midiLearn();

public:

	gdMidiGrabber(class Channel *ch);
	~gdMidiGrabber();
};

#endif
