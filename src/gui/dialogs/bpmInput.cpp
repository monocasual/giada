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

#include "bpmInput.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/mixer.h"
#include "glue/main.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/input.h"
#include "mainWindow.h"
#include "utils/gui.h"
#include "utils/string.h"
#include <cstring>

extern giada::v::gdMainWindow* mainWin;

namespace giada::v
{
gdBpmInput::gdBpmInput(const char* label)
: gdWindow(u::gui::centerWindowX(144), u::gui::centerWindowY(36), 144, 36, "Bpm")
{
	set_modal();

	input_a = new geInput(8, 8, 30, G_GUI_UNIT);
	input_b = new geInput(42, 8, 20, G_GUI_UNIT);
	ok      = new geButton(66, 8, 70, G_GUI_UNIT, "Ok");
	end();

	std::vector<std::string> parts = u::string::split(label, ".");

	input_a->maximum_size(3);
	input_a->type(FL_INT_INPUT);
	input_a->value(parts[0].c_str());

	input_b->maximum_size(1);
	input_b->type(FL_INT_INPUT);
	input_b->value(parts[1].c_str());

	ok->shortcut(FL_Enter);
	ok->callback(cb_update, (void*)this);

	u::gui::setFavicon(this);
	setId(WID_BPM);
	show();
}

/* -------------------------------------------------------------------------- */

void gdBpmInput::cb_update(Fl_Widget* /*w*/, void* p) { ((gdBpmInput*)p)->cb_update(); }

/* -------------------------------------------------------------------------- */

void gdBpmInput::cb_update()
{
	if (strcmp(input_a->value(), "") == 0)
		return;
	c::main::setBpm(input_a->value(), input_b->value());
	do_callback();
}
} // namespace giada::v