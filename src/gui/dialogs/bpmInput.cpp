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

#include "gui/dialogs/bpmInput.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/mixer.h"
#include "glue/main.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/flex.h"
#include "gui/elems/basics/input.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include "utils/string.h"
#include <cstring>

extern giada::v::Ui g_ui;

namespace giada::v
{
gdBpmInput::gdBpmInput(const char* label)
: gdWindow(u::gui::getCenterWinBounds(180, 36), "Bpm")
{
	geFlex* container = new geFlex(getContentBounds().reduced({G_GUI_OUTER_MARGIN}), Direction::HORIZONTAL, G_GUI_INNER_MARGIN);
	{
		m_input_a = new geInput(0, 0, 0, 0);
		m_input_b = new geInput(0, 0, 0, 0);
		m_ok      = new geButton(g_ui.langMapper.get(LangMap::COMMON_OK));
		container->add(m_input_a);
		container->add(m_input_b);
		container->add(m_ok, 70);
		container->end();
	}

	add(container);

	std::vector<std::string> parts = u::string::split(label, ".");

	m_input_a->maximum_size(3);
	m_input_a->type(FL_INT_INPUT);
	m_input_a->value(parts[0].c_str());

	m_input_b->maximum_size(1);
	m_input_b->type(FL_INT_INPUT);
	m_input_b->value(parts[1].c_str());

	m_ok->shortcut(FL_Enter);
	m_ok->onClick = [this]() {
		if (strcmp(m_input_a->value(), "") == 0)
			return;
		c::main::setBpm(m_input_a->value(), m_input_b->value());
		do_callback();
	};

	set_modal();
	u::gui::setFavicon(this);
	setId(WID_BPM);
	show();
}
} // namespace giada::v