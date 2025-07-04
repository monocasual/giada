/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/gui/dialogs/bpmInput.h"
#include "src/glue/main.h"
#include "src/gui/elems/basics/flex.h"
#include "src/gui/elems/basics/input.h"
#include "src/gui/elems/basics/textButton.h"
#include "src/gui/ui.h"
#include "src/utils/gui.h"
#include "src/utils/string.h"
#include <cstring>
#include <fmt/core.h>

extern giada::v::Ui* g_ui;

namespace giada::v
{
gdBpmInput::gdBpmInput(float value)
: gdWindow(u::gui::getCenterWinBounds({-1, -1, 180, 36}), "Bpm", WID_BPM)
{
	geFlex* container = new geFlex(getContentBounds().reduced({G_GUI_OUTER_MARGIN}), Direction::HORIZONTAL, G_GUI_INNER_MARGIN);
	{
		m_value = new geInput(0, 0, 0, 0);
		m_ok    = new geTextButton(g_ui->getI18Text(LangMap::COMMON_OK));
		container->addWidget(m_value);
		container->addWidget(m_ok, 70);
		container->end();
	}

	add(container);

	m_value->setMaximumSize(5);
	m_value->setType(FL_FLOAT_INPUT);
	m_value->setValue(fmt::format("{:.1f}", value));

	m_ok->shortcut(FL_Enter);
	m_ok->onClick = [this]()
	{
		std::string value = m_value->getValue();
		if (value == "")
			return;
		c::main::setBpm(u::string::toFloat(value));
		do_callback();
	};

	set_modal();
	show();
}
} // namespace giada::v