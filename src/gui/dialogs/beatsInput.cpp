/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#include "beatsInput.h"
#include "core/const.h"
#include "glue/main.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/input.h"
#include "mainWindow.h"
#include "utils/gui.h"
#include "utils/string.h"
#include <cstring>

extern giada::v::gdMainWindow* mainWin;

namespace giada::v
{
gdBeatsInput::gdBeatsInput(int beats, int bars)
: gdWindow(u::gui::centerWindowX(180), u::gui::centerWindowY(36), 180, 36, "Beats")
{
	set_modal();

	m_beats = new geInput(8, 8, 43, G_GUI_UNIT);
	m_bars  = new geInput(m_beats->x() + m_beats->w() + 4, 8, 43, G_GUI_UNIT);
	m_ok    = new geButton(m_bars->x() + m_bars->w() + 4, 8, 70, G_GUI_UNIT, "Ok");
	end();

	m_beats->maximum_size(2);
	m_beats->value(std::to_string(beats).c_str());
	m_beats->type(FL_INT_INPUT);

	m_bars->maximum_size(2);
	m_bars->value(std::to_string(bars).c_str());
	m_bars->type(FL_INT_INPUT);

	m_ok->shortcut(FL_Enter);
	m_ok->callback(cb_update, (void*)this);

	u::gui::setFavicon(this);
	setId(WID_BEATS);
	show();
}

/* -------------------------------------------------------------------------- */

void gdBeatsInput::cb_update(Fl_Widget* /*w*/, void* p) { ((gdBeatsInput*)p)->cb_update(); }

/* -------------------------------------------------------------------------- */

void gdBeatsInput::cb_update()
{
	if (!strcmp(m_beats->value(), "") || !strcmp(m_bars->value(), ""))
		return;
	c::main::setBeats(atoi(m_beats->value()), atoi(m_bars->value()));
	do_callback();
}
} // namespace giada::v