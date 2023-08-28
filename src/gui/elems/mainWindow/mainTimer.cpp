/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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
#include "glue/layout.h"
#include "glue/main.h"
#include "gui/elems/basics/choice.h"
#include "gui/elems/basics/imageButton.h"
#include "gui/elems/basics/textButton.h"
#include "gui/graphics.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include <fmt/core.h>

extern giada::v::Ui* g_ui;

namespace giada::v
{
geMainTimer::geMainTimer()
: geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN)
{
	m_bpm        = new geTextButton("");
	m_meter      = new geTextButton("");
	m_quantizer  = new geChoice();
	m_multiplier = new geImageButton(graphics::multiplyOff, graphics::multiplyOn);
	m_divider    = new geImageButton(graphics::divideOff, graphics::divideOn);
	add(m_quantizer, 60);
	add(m_bpm, 60);
	add(m_meter, 60);
	add(m_multiplier, G_GUI_UNIT);
	add(m_divider, G_GUI_UNIT);
	end();

	m_bpm->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_TIMER_LABEL_BPM));
	m_meter->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_TIMER_LABEL_METER));
	m_quantizer->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_TIMER_LABEL_QUANTIZER));
	m_multiplier->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_TIMER_LABEL_MULTIPLIER));
	m_divider->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_TIMER_LABEL_DIVIDER));

	m_bpm->onClick        = [&timer = m_timer]() { c::layout::openBpmWindow(timer.bpm); };
	m_meter->onClick      = [&timer = m_timer]() { c::layout::openBeatsWindow(timer.beats, timer.bars); };
	m_multiplier->onClick = []() { c::main::multiplyBeats(); };
	m_divider->onClick    = []() { c::main::divideBeats(); };

	m_quantizer->addItem(g_ui->getI18Text(LangMap::COMMON_OFF));
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
	m_bpm->copy_label(fmt::format("{:.1f}", v).c_str()); // Only 1 decimal place (e.g. 120.0)
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
	m_meter->copy_label(fmt::format("{}/{}", beats, bars).c_str());
}
} // namespace giada::v
