
/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/gui/elems/sampleEditor/pitchTool.h"
#include "src/deps/mcl-utils/src/string.hpp"
#include "src/glue/channel.h"
#include "src/gui/dialogs/sampleEditor.h"
#include "src/gui/elems/basics/box.h"
#include "src/gui/elems/basics/choice.h"
#include "src/gui/elems/basics/dial.h"
#include "src/gui/elems/basics/input.h"
#include "src/gui/elems/basics/textButton.h"
#include "src/gui/graphics.h"
#include "src/gui/ui.h"
#include "src/utils/gui.h"
#include <fmt/core.h>

extern giada::v::Ui* g_ui;

namespace utils = mcl::utils;

namespace giada::v
{
gePitchTool::gePitchTool(const c::sampleEditor::Data& d)
: geFlex(Direction::VERTICAL, G_GUI_INNER_MARGIN)
, m_data(nullptr)
{
	geFlex* row1 = new geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN);
	{
		m_playbackModeLabel = new geBox("Mode", FL_ALIGN_LEFT);
		m_playbackMode      = new geChoice();

		row1->addWidget(m_playbackModeLabel, 50);
		row1->addWidget(m_playbackMode, 70);
		row1->end();
	}

	geFlex* row2 = new geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN);
	{
		m_pitchLabel  = new geBox(g_ui->getI18Text(LangMap::SAMPLEEDITOR_PITCH), FL_ALIGN_LEFT);
		m_pitch       = new geInput();
		m_pitchToBar  = new geTextButton(g_ui->getI18Text(LangMap::SAMPLEEDITOR_PITCH_TOBAR));
		m_pitchToSong = new geTextButton(g_ui->getI18Text(LangMap::SAMPLEEDITOR_PITCH_TOSONG));
		row2->addWidget(m_pitchLabel, 50);
		row2->addWidget(m_pitch, 70);
		row2->addWidget(m_pitchToBar, 70);
		row2->addWidget(m_pitchToSong, 70);
		row2->end();
	}

	geFlex* row3 = new geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN);
	{
		m_timeLabel = new geBox("Time", FL_ALIGN_LEFT);
		m_time      = new geInput();
		row3->addWidget(m_timeLabel, 50);
		row3->addWidget(m_time, 70);
		row3->end();
	}

	addWidget(row1, G_GUI_UNIT);
	addWidget(row2, G_GUI_UNIT);
	addWidget(row3, G_GUI_UNIT);
	end();

	m_pitch->setType(FL_FLOAT_INPUT);
	m_pitch->setWhen(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY); // on focus lost or enter key
	m_pitch->onChange = [this](const std::string& val)
	{
		c::channel::setChannelPitch(m_data->channelId, utils::string::toFloat(val), Thread::MAIN);
	};

	m_pitchToBar->onClick = [this]()
	{
		const float pitch = m_data->getSample().range.getLength() / (float)m_data->getFramesInBar();
		c::channel::setChannelPitch(m_data->channelId, pitch, Thread::MAIN);
	};

	m_pitchToSong->onClick = [this]()
	{
		const float pitch = m_data->getSample().range.getLength() / (float)m_data->getFramesInLoop();
		c::channel::setChannelPitch(m_data->channelId, pitch, Thread::MAIN);
	};

	m_playbackMode->addItem("Tape", static_cast<int>(PlaybackMode::TAPE));
	m_playbackMode->addItem("Elastic", static_cast<int>(PlaybackMode::ELASTIC));
	m_playbackMode->onChange = [this](int id)
	{
		c::channel::setChannelPlaybackMode(m_data->channelId, static_cast<PlaybackMode>(id));
		updateInputStates();
	};

	m_time->setType(FL_FLOAT_INPUT);
	m_time->setWhen(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY); // on focus lost or enter key
	m_time->onChange = [this](const std::string& val)
	{
		c::channel::setChannelTime(m_data->channelId, utils::string::toFloat(val));
	};

	rebuild(d);
}

/* -------------------------------------------------------------------------- */

void gePitchTool::rebuild(const c::sampleEditor::Data& d)
{
	m_data = &d;
	m_playbackMode->showItem(static_cast<int>(m_data->getSample().playbackMode));
	m_pitch->setValue(fmt::format("{:.4f}", m_data->getSample().pitch)); // 4 digits
	m_time->setValue(fmt::format("{:.4f}", m_data->getSample().time));   // 4 digits
	updateInputStates();
}

/* -------------------------------------------------------------------------- */

void gePitchTool::refresh()
{
	m_pitch->setValue(fmt::format("{:.4f}", m_data->getSample().pitch)); // 4 digits
}

/* -------------------------------------------------------------------------- */

void gePitchTool::updateInputStates()
{
	const auto currentPlaybackMode = static_cast<PlaybackMode>(m_playbackMode->getSelectedId());
	u::gui::setActive(m_time, currentPlaybackMode == PlaybackMode::ELASTIC);
}
} // namespace giada::v