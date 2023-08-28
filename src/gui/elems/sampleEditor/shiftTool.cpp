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

#include "gui/elems/sampleEditor/shiftTool.h"
#include "core/const.h"
#include "glue/sampleEditor.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/basics/textButton.h"
#include "gui/ui.h"
#include "utils/string.h"

extern giada::v::Ui* g_ui;

namespace giada::v
{
geShiftTool::geShiftTool(const c::sampleEditor::Data& d)
: geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN)
, m_data(nullptr)
{
	m_label = new geBox(g_ui->getI18Text(LangMap::SAMPLEEDITOR_SHIFT), FL_ALIGN_LEFT);
	m_shift = new geInput();
	m_reset = new geTextButton(g_ui->getI18Text(LangMap::COMMON_RESET));
	add(m_label, 50);
	add(m_shift, 70);
	add(m_reset, 70);
	end();

	m_shift->setType(FL_INT_INPUT);
	m_shift->setWhen(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY); // on focus lost or enter key
	m_shift->onChange = [this](const std::string& val) {
		c::sampleEditor::shift(m_data->channelId, u::string::toInt(val));
	};

	m_reset->onClick = [this]() {
		c::sampleEditor::shift(m_data->channelId, 0);
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
	m_shift->setValue(std::to_string(shift));
}
} // namespace giada::v