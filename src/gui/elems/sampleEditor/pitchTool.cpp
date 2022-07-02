
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

#include "pitchTool.h"
#include "core/const.h"
#include "core/graphics.h"
#include "core/model/model.h"
#include "glue/events.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/input.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include "utils/string.h"
#include <FL/Fl.H>

extern giada::v::Ui g_ui;

namespace giada::v
{
gePitchTool::gePitchTool(const c::sampleEditor::Data& d, int x, int y)
: gePack(x, y, Direction::HORIZONTAL)
, m_data(nullptr)
, m_label(0, 0, 60, G_GUI_UNIT, g_ui.langMapper.get(LangMap::SAMPLEEDITOR_PITCH), FL_ALIGN_LEFT)
, m_dial(0, 0, G_GUI_UNIT, G_GUI_UNIT)
, m_input(0, 0, 70, G_GUI_UNIT)
, m_pitchToBar(0, 0, 70, G_GUI_UNIT, g_ui.langMapper.get(LangMap::SAMPLEEDITOR_PITCH_TOBAR))
, m_pitchToSong(0, 0, 70, G_GUI_UNIT, g_ui.langMapper.get(LangMap::SAMPLEEDITOR_PITCH_TOSONG))
, m_pitchHalf(0, 0, G_GUI_UNIT, G_GUI_UNIT, "", divideOff_xpm, divideOn_xpm)
, m_pitchDouble(0, 0, G_GUI_UNIT, G_GUI_UNIT, "", multiplyOff_xpm, multiplyOn_xpm)
, m_pitchReset(0, 0, 70, G_GUI_UNIT, g_ui.langMapper.get(LangMap::COMMON_RESET))
{
	add(&m_label);
	add(&m_dial);
	add(&m_input);
	add(&m_pitchToBar);
	add(&m_pitchToSong);
	add(&m_pitchHalf);
	add(&m_pitchDouble);
	add(&m_pitchReset);

	m_dial.range(0.01f, 4.0f);
	m_dial.callback(cb_setPitch, (void*)this);
	m_dial.when(FL_WHEN_RELEASE);

	m_input.onChange = [this](const std::string& val) {
		c::events::setChannelPitch(m_data->channelId, val == "" ? 0 : std::stof(val), Thread::MAIN);
	};

	m_pitchToBar.callback(cb_setPitchToBar, (void*)this);
	m_pitchToSong.callback(cb_setPitchToSong, (void*)this);
	m_pitchHalf.callback(cb_setPitchHalf, (void*)this);
	m_pitchDouble.callback(cb_setPitchDouble, (void*)this);
	m_pitchReset.callback(cb_resetPitch, (void*)this);

	rebuild(d);
}

/* -------------------------------------------------------------------------- */

void gePitchTool::rebuild(const c::sampleEditor::Data& d)
{
	m_data = &d;
	update(m_data->pitch, /*isDial=*/false);
}

/* -------------------------------------------------------------------------- */

void gePitchTool::update(float v, bool isDial)
{
	m_input.setValue(u::string::fToString(v, 4)); // 4 digits
	if (!isDial)
		m_dial.value(v);
}

/* -------------------------------------------------------------------------- */

void gePitchTool::cb_setPitch(Fl_Widget* /*w*/, void* p) { ((gePitchTool*)p)->cb_setPitch(); }
void gePitchTool::cb_setPitchToBar(Fl_Widget* /*w*/, void* p) { ((gePitchTool*)p)->cb_setPitchToBar(); }
void gePitchTool::cb_setPitchToSong(Fl_Widget* /*w*/, void* p) { ((gePitchTool*)p)->cb_setPitchToSong(); }
void gePitchTool::cb_setPitchHalf(Fl_Widget* /*w*/, void* p) { ((gePitchTool*)p)->cb_setPitchHalf(); }
void gePitchTool::cb_setPitchDouble(Fl_Widget* /*w*/, void* p) { ((gePitchTool*)p)->cb_setPitchDouble(); }
void gePitchTool::cb_resetPitch(Fl_Widget* /*w*/, void* p) { ((gePitchTool*)p)->cb_resetPitch(); }

/* -------------------------------------------------------------------------- */

void gePitchTool::cb_setPitch()
{
	c::events::setChannelPitch(m_data->channelId, m_dial.value(), Thread::MAIN);
}

/* -------------------------------------------------------------------------- */

void gePitchTool::cb_setPitchHalf()
{
	c::events::setChannelPitch(m_data->channelId, m_dial.value() / 2, Thread::MAIN);
}

/* -------------------------------------------------------------------------- */

void gePitchTool::cb_setPitchDouble()
{
	c::events::setChannelPitch(m_data->channelId, m_dial.value() * 2, Thread::MAIN);
}

/* -------------------------------------------------------------------------- */

void gePitchTool::cb_setPitchToBar()
{
	c::events::setChannelPitch(m_data->channelId, m_data->end / (float)m_data->getFramesInBar(),
	    Thread::MAIN);
}

/* -------------------------------------------------------------------------- */

void gePitchTool::cb_setPitchToSong()
{
	c::events::setChannelPitch(m_data->channelId, m_data->end / (float)m_data->getFramesInLoop(),
	    Thread::MAIN);
}

/* -------------------------------------------------------------------------- */

void gePitchTool::cb_resetPitch()
{
	c::events::setChannelPitch(m_data->channelId, G_DEFAULT_PITCH, Thread::MAIN);
}
} // namespace giada::v