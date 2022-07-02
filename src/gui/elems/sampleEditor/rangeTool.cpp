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

#include "rangeTool.h"
#include "core/model/model.h"
#include "core/wave.h"
#include "glue/channel.h"
#include "glue/sampleEditor.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include "utils/string.h"
#include "waveTools.h"
#include <FL/Fl.H>
#include <cassert>

extern giada::v::Ui g_ui;

namespace giada::v
{
geRangeTool::geRangeTool(const c::sampleEditor::Data& d, int x, int y)
: gePack(x, y, Direction::HORIZONTAL)
, m_data(nullptr)
, m_label(0, 0, 60, G_GUI_UNIT, g_ui.langMapper.get(LangMap::SAMPLEEDITOR_RANGE), FL_ALIGN_LEFT)
, m_begin(0, 0, 70, G_GUI_UNIT)
, m_end(0, 0, 70, G_GUI_UNIT)
, m_reset(0, 0, 70, G_GUI_UNIT, g_ui.langMapper.get(LangMap::COMMON_RESET))
{
	add(&m_label);
	add(&m_begin);
	add(&m_end);
	add(&m_reset);

	m_begin.setType(FL_INT_INPUT);
	m_begin.when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY); // on focus lost or enter key
	m_begin.onChange = [this](const std::string& val) {
		c::sampleEditor::setBeginEnd(m_data->channelId, val == "" ? 0 : std::stoi(val), std::stoi(m_end.getValue()));
	};

	m_end.setType(FL_INT_INPUT);
	m_end.when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY); // on focus lost or enter key
	m_end.onChange = [this](const std::string& val) {
		c::sampleEditor::setBeginEnd(m_data->channelId, std::stoi(m_begin.getValue()), val == "" ? 0 : std::stoi(val));
	};

	m_reset.onClick = [this]() {
		c::sampleEditor::setBeginEnd(m_data->channelId, 0, m_data->waveSize - 1);
	};

	rebuild(d);
}

/* -------------------------------------------------------------------------- */

void geRangeTool::rebuild(const c::sampleEditor::Data& d)
{
	m_data = &d;
	update(m_data->begin, m_data->end);
}

/* -------------------------------------------------------------------------- */

void geRangeTool::update(Frame begin, Frame end)
{
	m_begin.setValue(std::to_string(begin));
	m_end.setValue(std::to_string(end));
}
} // namespace giada::v