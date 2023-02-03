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

#include "gui/dialogs/bpmInput.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/mixer.h"
#include "glue/events.h"
#include "gui/elems/basics/flex.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/basics/textButton.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include "utils/string.h"
#include <cstring>

extern giada::v::Ui g_ui;

namespace giada::v
{
gdBpmInput::gdBpmInput(const char* label)
: gdWindow(u::gui::getCenterWinBounds({-1, -1, 180, 36}), "Bpm")
{
	geFlex* container = new geFlex(getContentBounds().reduced({G_GUI_OUTER_MARGIN}), Direction::HORIZONTAL, G_GUI_INNER_MARGIN);
	{
		m_input_a = new geInput(0, 0, 0, 0);
		m_input_b = new geInput(0, 0, 0, 0);
		m_ok      = new geTextButton(g_ui.getI18Text(LangMap::COMMON_OK));
		container->add(m_input_a);
		container->add(m_input_b);
		container->add(m_ok, 70);
		container->end();
	}

	add(container);

	std::vector<std::string> parts = u::string::split(label, ".");

	m_input_a->setMaximumSize(3);
	m_input_a->setType(FL_INT_INPUT);
	m_input_a->setValue(parts[0]);

	m_input_b->setMaximumSize(1);
	m_input_b->setType(FL_INT_INPUT);
	m_input_b->setValue(parts[1]);

	m_ok->shortcut(FL_Enter);
	m_ok->onClick = [this]() {
		if (m_input_a->getValue() == "")
			return;
		const std::string a   = m_input_a->getValue();
		const std::string b   = m_input_b->getValue();
		const float       bpm = u::gui::toFloat(a) + (u::gui::toFloat(b) / 10.0f);
		c::events::setBpm(bpm);
		do_callback();
	};

	set_modal();
	setId(WID_BPM);
	show();
}
} // namespace giada::v