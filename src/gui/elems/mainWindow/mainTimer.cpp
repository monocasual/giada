/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "mainTimer.h"
#include "core/const.h"
#include "core/graphics.h"
#include "glue/events.h"
#include "glue/layout.h"
#include "glue/main.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/choice.h"
#include "utils/gui.h"
#include "utils/string.h"

namespace giada::v
{
geMainTimer::geMainTimer(int x, int y)
: gePack(x, y, Direction::HORIZONTAL)
, m_bpm(0, 0, 60, G_GUI_UNIT)
, m_meter(0, 0, 60, G_GUI_UNIT)
, m_quantizer(0, 0, 60, G_GUI_UNIT, "", false)
, m_multiplier(0, 0, G_GUI_UNIT, G_GUI_UNIT, "", multiplyOff_xpm, multiplyOn_xpm)
, m_divider(0, 0, G_GUI_UNIT, G_GUI_UNIT, "", divideOff_xpm, divideOn_xpm)
{
	add(&m_quantizer);
	add(&m_bpm);
	add(&m_meter);
	add(&m_multiplier);
	add(&m_divider);

	resizable(nullptr); // don't resize any widget

	m_bpm.copy_tooltip("Beats per minute (BPM)");
	m_meter.copy_tooltip("Beats and bars");
	m_quantizer.copy_tooltip("Live quantizer");
	m_multiplier.copy_tooltip("Beat multiplier");
	m_divider.copy_tooltip("Beat divider");

	m_bpm.callback(cb_bpm, (void*)this);
	m_meter.callback(cb_meter, (void*)this);
	m_multiplier.callback(cb_multiplier, (void*)this);
	m_divider.callback(cb_divider, (void*)this);

	m_quantizer.add("off", 0, cb_quantizer, (void*)this);
	m_quantizer.add("1\\/1", 0, cb_quantizer, (void*)this);
	m_quantizer.add("1\\/2", 0, cb_quantizer, (void*)this);
	m_quantizer.add("1\\/3", 0, cb_quantizer, (void*)this);
	m_quantizer.add("1\\/4", 0, cb_quantizer, (void*)this);
	m_quantizer.add("1\\/6", 0, cb_quantizer, (void*)this);
	m_quantizer.add("1\\/8", 0, cb_quantizer, (void*)this);
	m_quantizer.value(0); //  "off" by default
}

/* -------------------------------------------------------------------------- */

void geMainTimer::cb_bpm(Fl_Widget* /*w*/, void* p) { ((geMainTimer*)p)->cb_bpm(); }
void geMainTimer::cb_meter(Fl_Widget* /*w*/, void* p) { ((geMainTimer*)p)->cb_meter(); }
void geMainTimer::cb_quantizer(Fl_Widget* /*w*/, void* p) { ((geMainTimer*)p)->cb_quantizer(); }
void geMainTimer::cb_multiplier(Fl_Widget* /*w*/, void* p) { ((geMainTimer*)p)->cb_multiplier(); }
void geMainTimer::cb_divider(Fl_Widget* /*w*/, void* p) { ((geMainTimer*)p)->cb_divider(); }

/* -------------------------------------------------------------------------- */

void geMainTimer::cb_bpm()
{
	c::layout::openBpmWindow(m_bpm.label());
}

/* -------------------------------------------------------------------------- */

void geMainTimer::cb_meter()
{
	c::layout::openBeatsWindow(m_timer.beats, m_timer.bars);
}

/* -------------------------------------------------------------------------- */

void geMainTimer::cb_quantizer()
{
	c::main::quantize(m_quantizer.value());
}

/* -------------------------------------------------------------------------- */

void geMainTimer::cb_multiplier()
{
	c::events::multiplyBeats();
}

/* -------------------------------------------------------------------------- */

void geMainTimer::cb_divider()
{
	c::events::divideBeats();
}

/* -------------------------------------------------------------------------- */

void geMainTimer::refresh()
{
	m_timer = c::main::getTimer();

	if (m_timer.isRecordingInput)
	{
		m_bpm.deactivate();
		m_meter.deactivate();
		m_multiplier.deactivate();
		m_divider.deactivate();
	}
	else
	{
		m_bpm.activate();
		m_meter.activate();
		m_multiplier.activate();
		m_divider.activate();
	}
}

/* -------------------------------------------------------------------------- */

void geMainTimer::rebuild()
{
	m_timer = c::main::getTimer();

	setBpm(m_timer.bpm);
	setMeter(m_timer.beats, m_timer.bars);
	setQuantizer(m_timer.quantize);
}

/* -------------------------------------------------------------------------- */

void geMainTimer::setBpm(const char* v)
{
	m_bpm.copy_label(v);
}

void geMainTimer::setBpm(float v)
{
	m_bpm.copy_label(u::string::fToString(v, 1).c_str()); // Only 1 decimal place (e.g. 120.0)
}

/* -------------------------------------------------------------------------- */

void geMainTimer::setLock(bool v)
{
	if (v)
	{
		m_bpm.deactivate();
		m_meter.deactivate();
		m_multiplier.deactivate();
		m_divider.deactivate();
	}
	else
	{
		m_bpm.activate();
		m_meter.activate();
		m_multiplier.activate();
		m_divider.activate();
	}
}

/* -------------------------------------------------------------------------- */

void geMainTimer::setQuantizer(int q)
{
	m_quantizer.value(q);
}

/* -------------------------------------------------------------------------- */

void geMainTimer::setMeter(int beats, int bars)
{
	std::string s = std::to_string(beats) + "/" + std::to_string(bars);
	m_meter.copy_label(s.c_str());
}
} // namespace giada::v
