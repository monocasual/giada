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

#include "shiftTool.h"
#include "core/const.h"
#include "core/model/model.h"
#include "glue/sampleEditor.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/dialogs/warnings.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include "utils/string.h"
#include <cassert>
#include <cstdlib>

extern giada::v::Ui g_ui;

namespace giada::v
{
geShiftTool::geShiftTool(const c::sampleEditor::Data& d, int x, int y)
: gePack(x, y, Direction::HORIZONTAL)
, m_data(nullptr)
, m_label(0, 0, 60, G_GUI_UNIT, g_ui.langMapper.get(LangMap::SAMPLEEDITOR_SHIFT), FL_ALIGN_LEFT)
, m_shift(0, 0, 70, G_GUI_UNIT)
, m_reset(0, 0, 70, G_GUI_UNIT, g_ui.langMapper.get(LangMap::COMMON_RESET))
{
	add(&m_label);
	add(&m_shift);
	add(&m_reset);

	m_shift.setType(FL_INT_INPUT);
	m_shift.when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY); // on focus lost or enter key
	m_shift.onChange = [this](const std::string& val) {
		shift(val == "" ? 0 : std::stoi(val));
	};

	m_reset.onClick = [this]() {
		shift(0);
	};

	rebuild(d);
}

/* -------------------------------------------------------------------------- */

void geShiftTool::rebuild(const c::sampleEditor::Data& d)
{
	m_data = &d;
	update(m_data->shift);
}

/* -------------------------------------------------------------------------- */

void geShiftTool::update(Frame shift)
{
	m_shift.setValue(std::to_string(shift));
}

/* -------------------------------------------------------------------------- */

void geShiftTool::shift(int f)
{
	c::sampleEditor::shift(m_data->channelId, f);
}
} // namespace giada::v