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

#include "gui/elems/sampleEditor/rangeTool.h"
#include "glue/channel.h"
#include "glue/sampleEditor.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/basics/pack.h"
#include "gui/elems/basics/textButton.h"
#include "gui/ui.h"
#include "utils/string.h"

extern giada::v::Ui* g_ui;

namespace giada::v
{
geRangeTool::geRangeTool(const c::sampleEditor::Data& d)
: geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN)
, m_data(nullptr)
{
	m_label = new geBox(g_ui->getI18Text(LangMap::SAMPLEEDITOR_RANGE), FL_ALIGN_LEFT);
	m_begin = new geInput();
	m_end   = new geInput();
	m_reset = new geTextButton(g_ui->getI18Text(LangMap::COMMON_RESET));
	addWidget(m_label, 50);
	addWidget(m_begin, 70);
	addWidget(m_end, 70);
	addWidget(m_reset, 70);
	end();

	m_begin->setType(FL_INT_INPUT);
	m_begin->setWhen(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY); // on focus lost or enter key
	m_begin->onChange = [this](const std::string& val) {
		c::sampleEditor::setBeginEnd(m_data->channelId, u::string::toInt(val), u::string::toInt(m_end->getValue()));
	};

	m_end->setType(FL_INT_INPUT);
	m_end->setWhen(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY); // on focus lost or enter key
	m_end->onChange = [this](const std::string& val) {
		c::sampleEditor::setBeginEnd(m_data->channelId, u::string::toInt(m_begin->getValue()), u::string::toInt(val));
	};

	m_reset->onClick = [this]() {
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
	m_begin->setValue(std::to_string(begin));
	m_end->setValue(std::to_string(end));
}
} // namespace giada::v