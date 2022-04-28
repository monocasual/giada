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

#include "gui/dialogs/beatsInput.h"
#include "core/const.h"
#include "glue/main.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/flex.h"
#include "gui/elems/basics/input.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include "utils/string.h"
#include <cstring>

extern giada::v::Ui g_ui;

namespace giada::v
{
gdBeatsInput::gdBeatsInput(int beats, int bars)
: gdWindow(u::gui::getCenterWinBounds(180, 36), "Beats")
{
	geFlex* container = new geFlex(getContentBounds().reduced({G_GUI_OUTER_MARGIN}), Direction::HORIZONTAL, G_GUI_INNER_MARGIN);
	{
		m_beats = new geInput(0, 0, 0, 0);
		m_bars  = new geInput(0, 0, 0, 0);
		m_ok    = new geButton(g_ui.langMapper.get(LangMap::COMMON_OK));
		container->add(m_beats);
		container->add(m_bars);
		container->add(m_ok, 70);
		container->end();
	}

	add(container);

	m_beats->maximum_size(2);
	m_beats->value(std::to_string(beats).c_str());
	m_beats->type(FL_INT_INPUT);

	m_bars->maximum_size(2);
	m_bars->value(std::to_string(bars).c_str());
	m_bars->type(FL_INT_INPUT);

	m_ok->shortcut(FL_Enter);
	m_ok->onClick = [this]() {
		if (!strcmp(m_beats->value(), "") || !strcmp(m_bars->value(), ""))
			return;
		c::main::setBeats(atoi(m_beats->value()), atoi(m_bars->value()));
		do_callback();
	};

	set_modal();
	u::gui::setFavicon(this);
	setId(WID_BEATS);
	show();
}
} // namespace giada::v