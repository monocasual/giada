/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
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


#include "../../../core/const.h"
#include "../../../core/mixer.h"
#include "../../../core/graphics.h"
#include "../../../core/clock.h"
#include "../../../glue/main.h"
#include "../../../utils/gui.h"
#include "../../../utils/string.h"
#include "../../elems/basics/button.h"
#include "../../elems/basics/choice.h"
#include "../../dialogs/gd_mainWindow.h"
#include "../../dialogs/bpmInput.h"
#include "../../dialogs/beatsInput.h"
#include "mainTimer.h"


extern gdMainWindow* G_MainWin;


using std::string;
using namespace giada;


geMainTimer::geMainTimer(int x, int y)
	: Fl_Group(x, y, 180, 20)
{
	begin();

	quantizer  = new geChoice(x, y, 40, 20, "", false);
	bpm        = new geButton(quantizer->x()+quantizer->w()+4,  y, 40, 20);
	meter      = new geButton(bpm->x()+bpm->w()+8,  y, 40, 20, "4/1");
	multiplier = new geButton(meter->x()+meter->w()+4, y, 20, 20, "", multiplyOff_xpm, multiplyOn_xpm);
	divider    = new geButton(multiplier->x()+multiplier->w()+4, y, 20, 20, "", divideOff_xpm, divideOn_xpm);

	end();

	resizable(nullptr);   // don't resize any widget

	bpm->copy_label(gu_fToString(m::clock::getBpm(), 1).c_str());
	bpm->callback(cb_bpm, (void*)this);

	meter->callback(cb_meter, (void*)this);
	
	multiplier->callback(cb_multiplier, (void*)this);
	
	divider->callback(cb_divider, (void*)this);

	quantizer->add("off", 0, cb_quantizer, (void*)this);
	quantizer->add("1\\/1", 0, cb_quantizer, (void*)this);
	quantizer->add("1\\/2", 0, cb_quantizer, (void*)this);
	quantizer->add("1\\/3", 0, cb_quantizer, (void*)this);
	quantizer->add("1\\/4", 0, cb_quantizer, (void*)this);
	quantizer->add("1\\/6", 0, cb_quantizer, (void*)this);
	quantizer->add("1\\/8", 0, cb_quantizer, (void*)this);
	quantizer->value(0); //  "off" by default
}


/* -------------------------------------------------------------------------- */


void geMainTimer::cb_bpm       (Fl_Widget* v, void* p) { ((geMainTimer*)p)->cb_bpm(); }
void geMainTimer::cb_meter     (Fl_Widget* v, void* p) { ((geMainTimer*)p)->cb_meter(); }
void geMainTimer::cb_quantizer (Fl_Widget* v, void* p) { ((geMainTimer*)p)->cb_quantizer(); }
void geMainTimer::cb_multiplier(Fl_Widget* v, void* p) { ((geMainTimer*)p)->cb_multiplier(); }
void geMainTimer::cb_divider   (Fl_Widget* v, void* p) { ((geMainTimer*)p)->cb_divider(); }


/* -------------------------------------------------------------------------- */


void geMainTimer::cb_bpm()
{
	gu_openSubWindow(G_MainWin, new gdBpmInput(bpm->label()), WID_BPM);
}


/* -------------------------------------------------------------------------- */


void geMainTimer::cb_meter()
{
	gu_openSubWindow(G_MainWin, new gdBeatsInput(), WID_BEATS);
}


/* -------------------------------------------------------------------------- */


void geMainTimer::cb_quantizer()
{
	c::main::quantize(quantizer->value());
}


/* -------------------------------------------------------------------------- */


void geMainTimer::cb_multiplier()
{
	c::main::beatsMultiply();
}


/* -------------------------------------------------------------------------- */


void geMainTimer::cb_divider()
{
	c::main::beatsDivide();
}


/* -------------------------------------------------------------------------- */


void geMainTimer::setBpm(const char* v)
{
	bpm->copy_label(v);
}


void geMainTimer::setBpm(float v)
{
	bpm->copy_label(gu_fToString((float) v, 1).c_str()); // Only 1 decimal place (e.g. 120.0)
}


/* -------------------------------------------------------------------------- */


void geMainTimer::setLock(bool v)
{
  if (v) {
    bpm->deactivate();
    meter->deactivate();
    multiplier->deactivate();
    divider->deactivate();
  }
  else {
    bpm->activate();
    meter->activate();
    multiplier->activate();
    divider->activate();
  }
}


/* -------------------------------------------------------------------------- */


void geMainTimer::setQuantizer(int q)
{
	quantizer->value(q);
}


/* -------------------------------------------------------------------------- */


void geMainTimer::setMeter(int beats, int bars)
{
	string tmp = gu_iToString(beats) + "/" + gu_iToString(bars);
	meter->copy_label(tmp.c_str());
}
