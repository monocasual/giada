
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

#include "gui/elems/sampleEditor/pitchTool.h"
#include "core/const.h"
#include "glue/channel.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/imageButton.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/basics/textButton.h"
#include "gui/graphics.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include "utils/string.h"
#include <fmt/core.h>

extern giada::v::Ui* g_ui;

namespace giada::v
{
gePitchTool::gePitchTool(const c::sampleEditor::Data& d)
: geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN)
, m_data(nullptr)
{
	m_label       = new geBox(g_ui->getI18Text(LangMap::SAMPLEEDITOR_PITCH), FL_ALIGN_LEFT);
	m_dial        = new geDial();
	m_input       = new geInput();
	m_pitchToBar  = new geTextButton(g_ui->getI18Text(LangMap::SAMPLEEDITOR_PITCH_TOBAR));
	m_pitchToSong = new geTextButton(g_ui->getI18Text(LangMap::SAMPLEEDITOR_PITCH_TOSONG));
	m_pitchHalf   = new geImageButton(graphics::divideOff, graphics::divideOn);
	m_pitchDouble = new geImageButton(graphics::multiplyOff, graphics::multiplyOn);
	m_pitchReset  = new geTextButton(g_ui->getI18Text(LangMap::COMMON_RESET));
	add(m_label, 50);
	add(m_dial, G_GUI_UNIT);
	add(m_input, 70);
	add(m_pitchToBar, 70);
	add(m_pitchToSong, 70);
	add(m_pitchHalf, G_GUI_UNIT);
	add(m_pitchDouble, G_GUI_UNIT);
	add(m_pitchReset, 70);
	end();

	m_dial->range(0.01f, 4.0f);
	m_dial->when(FL_WHEN_RELEASE);
	m_dial->onChange = [this](float val) {
		c::channel::setChannelPitch(m_data->channelId, val, Thread::MAIN);
	};

	m_input->setType(FL_FLOAT_INPUT);
	m_input->setWhen(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY); // on focus lost or enter key
	m_input->onChange = [this](const std::string& val) {
		c::channel::setChannelPitch(m_data->channelId, u::string::toFloat(val), Thread::MAIN);
	};

	m_pitchToBar->onClick = [this]() {
		c::channel::setChannelPitch(m_data->channelId, m_data->end / (float)m_data->getFramesInBar(),
		    Thread::MAIN);
	};

	m_pitchToSong->onClick = [this]() {
		c::channel::setChannelPitch(m_data->channelId, m_data->end / (float)m_data->getFramesInLoop(),
		    Thread::MAIN);
	};

	m_pitchHalf->onClick = [this]() {
		c::channel::setChannelPitch(m_data->channelId, m_dial->value() / 2, Thread::MAIN);
	};

	m_pitchDouble->onClick = [this]() {
		c::channel::setChannelPitch(m_data->channelId, m_dial->value() * 2, Thread::MAIN);
	};

	m_pitchReset->onClick = [this]() {
		c::channel::setChannelPitch(m_data->channelId, G_DEFAULT_PITCH, Thread::MAIN);
	};

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
	m_input->setValue(fmt::format("{:.4f}", v)); // 4 digits
	if (!isDial)
		m_dial->value(v);
}

} // namespace giada::v