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
{
	m_bpm        = new geButton(0, 0, 60, G_GUI_UNIT);
	m_meter      = new geButton(0, 0, 60, G_GUI_UNIT);
	m_quantizer  = new geChoice(0, 0, 60, G_GUI_UNIT);
	m_multiplier = new geButton(0, 0, G_GUI_UNIT, G_GUI_UNIT, "", multiplyOff_xpm, multiplyOn_xpm);
	m_divider    = new geButton(0, 0, G_GUI_UNIT, G_GUI_UNIT, "", divideOff_xpm, divideOn_xpm);
	add(m_quantizer);
	add(m_bpm);
	add(m_meter);
	add(m_multiplier);
	add(m_divider);

	resizable(nullptr); // don't resize any widget

	m_bpm->copy_tooltip("Beats per minute (BPM)");
	m_meter->copy_tooltip("Beats and bars");
	m_quantizer->copy_tooltip("Live quantizer");
	m_multiplier->copy_tooltip("Beat multiplier");
	m_divider->copy_tooltip("Beat divider");

	m_bpm->onClick        = [&bpm = m_bpm]() { c::layout::openBpmWindow(bpm->label()); };
	m_meter->onClick      = [&timer = m_timer]() { c::layout::openBeatsWindow(timer.beats, timer.bars); };
	m_multiplier->onClick = []() { c::events::multiplyBeats(); };
	m_divider->onClick    = []() { c::events::divideBeats(); };

	m_quantizer->addItem("off");
	m_quantizer->addItem("1\\/1");
	m_quantizer->addItem("1\\/2");
	m_quantizer->addItem("1\\/3");
	m_quantizer->addItem("1\\/4");
	m_quantizer->addItem("1\\/6");
	m_quantizer->addItem("1\\/8");
	m_quantizer->showItem(1); //  "off" by default
	m_quantizer->onChange = [](ID value) { c::main::quantize(value); };
}

/* -------------------------------------------------------------------------- */

void geMainTimer::refresh()
{
	m_timer = c::main::getTimer();

	if (m_timer.isRecordingInput)
	{
		m_bpm->deactivate();
		m_meter->deactivate();
		m_multiplier->deactivate();
		m_divider->deactivate();
	}
	else
	{
		m_bpm->activate();
		m_meter->activate();
		m_multiplier->activate();
		m_divider->activate();
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
	m_bpm->copy_label(v);
}

void geMainTimer::setBpm(float v)
{
	m_bpm->copy_label(u::string::fToString(v, 1).c_str()); // Only 1 decimal place (e.g. 120.0)
}

/* -------------------------------------------------------------------------- */

void geMainTimer::setLock(bool v)
{
	if (v)
	{
		m_bpm->deactivate();
		m_meter->deactivate();
		m_multiplier->deactivate();
		m_divider->deactivate();
	}
	else
	{
		m_bpm->activate();
		m_meter->activate();
		m_multiplier->activate();
		m_divider->activate();
	}
}

/* -------------------------------------------------------------------------- */

void geMainTimer::setQuantizer(int q)
{
	m_quantizer->showItem(q);
}

/* -------------------------------------------------------------------------- */

void geMainTimer::setMeter(int beats, int bars)
{
	std::string s = std::to_string(beats) + "/" + std::to_string(bars);
	m_meter->copy_label(s.c_str());
}
} // namespace giada::v
