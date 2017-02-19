/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * mainTimer
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


#include "../../../core/const.h"
#include "../../../core/mixer.h"
#include "../../../core/graphics.h"
#include "../../../core/clock.h"
#include "../../../glue/main.h"
#include "../../../utils/gui.h"
#include "../../elems/ge_mixed.h"
#include "../../dialogs/gd_mainWindow.h"
#include "../../dialogs/gd_bpmInput.h"
#include "../../dialogs/gd_beatsInput.h"
#include "mainTimer.h"


extern Mixer         G_Mixer;
extern Clock         G_Clock;
extern gdMainWindow *G_MainWin;


geMainTimer::geMainTimer(int x, int y)
	: Fl_Group(x, y, 180, 20)
{
	begin();

	quantizer  = new gChoice(x, y, 40, 20, "", false);
	bpm        = new gClick (quantizer->x()+quantizer->w()+4,  y, 40, 20);
	meter      = new gClick (bpm->x()+bpm->w()+8,  y, 40, 20, "4/1");
	multiplier = new gClick (meter->x()+meter->w()+4, y, 20, 20, "", multiplyOff_xpm, multiplyOn_xpm);
	divider    = new gClick (multiplier->x()+multiplier->w()+4, y, 20, 20, "", divideOff_xpm, divideOn_xpm);

	end();

	resizable(NULL);   // don't resize any widget

	char buf[6]; snprintf(buf, 6, "%f", G_Clock.getBpm());
	bpm->copy_label(buf);

	bpm->callback(cb_bpm, (void*)this);
	meter->callback(cb_meter, (void*)this);
	multiplier->callback(cb_multiplier, (void*)this);
	divider->callback(cb_divider, (void*)this);

	quantizer->add("off", 0, cb_quantizer, (void*)this);
	quantizer->add("1b",  0, cb_quantizer, (void*)this);
	quantizer->add("2b",  0, cb_quantizer, (void*)this);
	quantizer->add("3b",  0, cb_quantizer, (void*)this);
	quantizer->add("4b",  0, cb_quantizer, (void*)this);
	quantizer->add("6b",  0, cb_quantizer, (void*)this);
	quantizer->add("8b",  0, cb_quantizer, (void*)this);
	quantizer->value(0); //  "off" by default
}


/* -------------------------------------------------------------------------- */


void geMainTimer::cb_bpm       (Fl_Widget *v, void *p) { ((geMainTimer*)p)->__cb_bpm(); }
void geMainTimer::cb_meter     (Fl_Widget *v, void *p) { ((geMainTimer*)p)->__cb_meter(); }
void geMainTimer::cb_quantizer (Fl_Widget *v, void *p) { ((geMainTimer*)p)->__cb_quantizer(); }
void geMainTimer::cb_multiplier(Fl_Widget *v, void *p) { ((geMainTimer*)p)->__cb_multiplier(); }
void geMainTimer::cb_divider   (Fl_Widget *v, void *p) { ((geMainTimer*)p)->__cb_divider(); }


/* -------------------------------------------------------------------------- */


void geMainTimer::__cb_bpm()
{
	gu_openSubWindow(G_MainWin, new gdBpmInput(bpm->label()), WID_BPM);
}


/* -------------------------------------------------------------------------- */


void geMainTimer::__cb_meter()
{
	gu_openSubWindow(G_MainWin, new gdBeatsInput(), WID_BEATS);
}


/* -------------------------------------------------------------------------- */


void geMainTimer::__cb_quantizer()
{
	glue_quantize(quantizer->value());
}


/* -------------------------------------------------------------------------- */


void geMainTimer::__cb_multiplier()
{
	glue_beatsMultiply();
}


/* -------------------------------------------------------------------------- */


void geMainTimer::__cb_divider()
{
	glue_beatsDivide();
}


/* -------------------------------------------------------------------------- */


void geMainTimer::setBpm(const char *v)
{
	bpm->copy_label(v);
}


void geMainTimer::setBpm(float v)
{
	char buf[6];
	sprintf(buf, "%.01f", v);  // only 1 decimal place (e.g. 120.0)
	bpm->copy_label(buf);
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


void geMainTimer::setMeter(int beats, int bars)
{
	char buf[8];
	sprintf(buf, "%d/%d", beats, bars);
	meter->copy_label(buf);
}
