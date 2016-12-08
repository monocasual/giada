/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * midiOutputSampleCh
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


#include "../../../core/sampleChannel.h"
#include "../../../utils/gui.h"
#include "../../elems/ge_mixed.h"
#include "../../elems/midiLearner.h"
#include "midiOutputSampleCh.h"


gdMidiOutputSampleCh::gdMidiOutputSampleCh(SampleChannel *ch)
	: gdMidiOutputBase(300, 140), ch(ch)
{
	setTitle(ch->index+1);

	enableLightning = new gCheck(8, 8, 120, 20, "Enable MIDI lightning output");
	new geMidiLearner(8, enableLightning->y()+enableLightning->h()+8, w()-16, "playing", cb_learn, &ch->midiOutLplaying);
	new geMidiLearner(8, enableLightning->y()+enableLightning->h()+32, w()-16, "mute",   cb_learn, &ch->midiOutLmute);
	new geMidiLearner(8, enableLightning->y()+enableLightning->h()+56, w()-16, "solo",   cb_learn, &ch->midiOutLsolo);

	close = new gButton(w()-88, enableLightning->y()+enableLightning->h()+84, 80, 20, "Close");
	close->callback(cb_close, (void*)this);

	enableLightning->value(ch->midiOutL);
	enableLightning->callback(cb_enableLightning, (void*)this);

	set_modal();
	gu_setFavicon(this);
	show();
}


/* -------------------------------------------------------------------------- */


void gdMidiOutputSampleCh::cb_close(Fl_Widget *w, void *p) { ((gdMidiOutputSampleCh*)p)->__cb_close(); }


/* -------------------------------------------------------------------------- */


void gdMidiOutputSampleCh::__cb_close()
{
	ch->midiOutL = enableLightning->value();
	do_callback();
}
