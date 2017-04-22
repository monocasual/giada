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


#include "../../../utils/gui.h"
#include "../../../core/conf.h"
#include "../../elems/midiLearner.h"
#include "../../elems/basics/button.h"
#include "midiInputMaster.h"


using namespace giada;


gdMidiInputMaster::gdMidiInputMaster()
	: gdMidiInputBase(0, 0, 300, 256, "MIDI Input Setup (global)")
{
	set_modal();

	new geMidiLearner(8,   8, w()-16, "rewind",           &cb_learn, &conf::midiInRewind);
	new geMidiLearner(8,  32, w()-16, "play/stop",        &cb_learn, &conf::midiInStartStop);
	new geMidiLearner(8,  56, w()-16, "action recording", &cb_learn, &conf::midiInActionRec);
	new geMidiLearner(8,  80, w()-16, "input recording",  &cb_learn, &conf::midiInInputRec);
	new geMidiLearner(8, 104, w()-16, "metronome",        &cb_learn, &conf::midiInMetronome);
	new geMidiLearner(8, 128, w()-16, "input volume",     &cb_learn, &conf::midiInVolumeIn);
	new geMidiLearner(8, 152, w()-16, "output volume",    &cb_learn, &conf::midiInVolumeOut);
	new geMidiLearner(8, 176, w()-16, "sequencer ×2",     &cb_learn, &conf::midiInBeatDouble);
	new geMidiLearner(8, 200, w()-16, "sequencer ÷2",     &cb_learn, &conf::midiInBeatHalf);
	ok = new geButton(w()-88, 228, 80, 20, "Close");

	ok->callback(cb_close, (void*)this);

	gu_setFavicon(this);
	show();
}
