/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#include "core/const.h"
#include "core/model/model.h"
#include "core/kernelAudio.h"
#include "core/mixer.h"
#include "core/recManager.h"
#include "core/graphics.h"
#include "core/clock.h"
#include "glue/main.h"
#include "utils/gui.h"
#include "utils/string.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/choice.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/bpmInput.h"
#include "gui/dialogs/beatsInput.h"
#include "mainTimer.h"


extern giada::v::gdMainWindow* G_MainWin;


namespace giada {
namespace v
{
geMainTimer::geMainTimer(int x, int y)
	: Fl_Group(x, y, 210, 20)
{
	begin();

	quantizer  = new geChoice(x, y, 50, 20, "", false);
	bpm        = new geButton(quantizer->x()+quantizer->w()+4,  y, 50, 20);
	meter      = new geButton(bpm->x()+bpm->w()+8,  y, 50, 20);
	multiplier = new geButton(meter->x()+meter->w()+4, y, 20, 20, "", multiplyOff_xpm, multiplyOn_xpm);
	divider    = new geButton(multiplier->x()+multiplier->w()+4, y, 20, 20, "", divideOff_xpm, divideOn_xpm);

	end();

	resizable(nullptr);   // don't resize any widget

	bpm->copy_label(u::string::fToString(m::clock::getBpm(), 1).c_str());
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

#if defined(G_OS_LINUX) || defined(G_OS_FREEBSD)
	
	/* Can't change bpm from within Giada when using JACK. */

	if (m::kernelAudio::getAPI() == G_SYS_API_JACK)
		bpm->deactivate();

#endif
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
	u::gui::openSubWindow(G_MainWin, new gdBpmInput(bpm->label()), WID_BPM);
}


/* -------------------------------------------------------------------------- */


void geMainTimer::cb_meter()
{
	u::gui::openSubWindow(G_MainWin, new gdBeatsInput(), WID_BEATS);
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


void geMainTimer::refresh()
{
	if (m::recManager::isRecordingInput()) {
		bpm->deactivate();
		meter->deactivate();
		multiplier->deactivate();
		divider->deactivate();
	}
	else {
		/* Don't reactivate bpm when using JACK. It must stay disabled. */

#if defined(G_OS_LINUX) || defined(G_OS_FREEBSD)
		if (m::kernelAudio::getAPI() != G_SYS_API_JACK)
			bpm->activate();
#else
		bpm->activate();
#endif
		meter->activate();	
		multiplier->activate();
		divider->activate();	
	}
}


/* -------------------------------------------------------------------------- */


void geMainTimer::rebuild()
{
	m::model::onGet(m::model::clock, [&](m::model::Clock& c)
	{
		setBpm(c.bpm);
		setMeter(c.beats, c.bars);
		setQuantizer(c.quantize);
	});
}


/* -------------------------------------------------------------------------- */


void geMainTimer::setBpm(const char* v)
{
	bpm->copy_label(v);
}


void geMainTimer::setBpm(float v)
{
	bpm->copy_label(u::string::fToString((float) v, 1).c_str()); // Only 1 decimal place (e.g. 120.0)
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
	std::string s = std::to_string(beats) + "/" + std::to_string(bars);
	meter->copy_label(s.c_str());
}

}} // giada::v::
