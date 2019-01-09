
/* -----------------------------------------------------------------------------
 *
, ch * Giada - Your Hardcore Loopmachine
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


#include "../../../core/midiChannel.h"
#include "../../../utils/gui.h"
#include "../../elems/midiLearner.h"
#include "../../elems/basics/button.h"
#include "../../elems/basics/check.h"
#include "../../elems/basics/choice.h"
#include "../../elems/mainWindow/keyboard/channel.h"
#include "midiOutputMidiCh.h"


using namespace giada;


gdMidiOutputMidiCh::gdMidiOutputMidiCh(m::MidiChannel* ch)
	: gdMidiOutputBase(300, 168), ch(ch)
{
	setTitle(ch->index+1);
	begin();

	enableOut   = new geCheck(x()+8, y()+8, 150, 20, "Enable MIDI output");
	chanListOut = new geChoice(w()-108, y()+8, 100, 20);

	enableLightning = new geCheck(x()+8, chanListOut->y()+chanListOut->h()+8, 120, 20, "Enable MIDI lightning output");
	new geMidiLearner(x()+8, enableLightning->y()+enableLightning->h()+8,  w()-16, "playing", 
		cb_learn, &ch->midiOutLplaying, ch);
	new geMidiLearner(x()+8, enableLightning->y()+enableLightning->h()+32, w()-16, "mute",    
		cb_learn, &ch->midiOutLmute, ch);
	new geMidiLearner(x()+8, enableLightning->y()+enableLightning->h()+56, w()-16, "solo",    
		cb_learn, &ch->midiOutLsolo, ch);

	close = new geButton(w()-88, enableLightning->y()+enableLightning->h()+84, 80, 20, "Close");

	end();

	chanListOut->add("Channel 1");
	chanListOut->add("Channel 2");
	chanListOut->add("Channel 3");
	chanListOut->add("Channel 4");
	chanListOut->add("Channel 5");
	chanListOut->add("Channel 6");
	chanListOut->add("Channel 7");
	chanListOut->add("Channel 8");
	chanListOut->add("Channel 9");
	chanListOut->add("Channel 10");
	chanListOut->add("Channel 11");
	chanListOut->add("Channel 12");
	chanListOut->add("Channel 13");
	chanListOut->add("Channel 14");
	chanListOut->add("Channel 15");
	chanListOut->add("Channel 16");
	chanListOut->value(0);

	if (ch->midiOut)
		enableOut->value(1);
	else
		chanListOut->deactivate();

	if (ch->midiOutL)
		enableLightning->value(1);

	chanListOut->value(ch->midiOutChan);

	enableOut->callback(cb_enableChanList, (void*)this);
	close->callback(cb_close, (void*)this);

	set_modal();
	gu_setFavicon(this);
	show();
}


/* -------------------------------------------------------------------------- */


void gdMidiOutputMidiCh::cb_close         (Fl_Widget *w, void *p) { ((gdMidiOutputMidiCh*)p)->__cb_close(); }
void gdMidiOutputMidiCh::cb_enableChanList(Fl_Widget *w, void *p) { ((gdMidiOutputMidiCh*)p)->__cb_enableChanList(); }


/* -------------------------------------------------------------------------- */


void gdMidiOutputMidiCh::__cb_enableChanList()
{
	enableOut->value() ? chanListOut->activate() : chanListOut->deactivate();
}


/* -------------------------------------------------------------------------- */


void gdMidiOutputMidiCh::__cb_close()
{
	ch->midiOut     = enableOut->value();
	ch->midiOutChan = chanListOut->value();
	ch->midiOutL    = enableLightning->value();
	ch->guiChannel->update();
	do_callback();
}
