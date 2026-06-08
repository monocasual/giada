/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2026 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/gui/elems/mainWindow/cpuLoad.h"
#include "src/glue/main.h"
#include "src/gui/const.h"
#include "src/gui/drawing.h"
#include "src/gui/elems/basics/box.h"
#include "src/gui/elems/basics/progress.h"
#include <fmt/core.h>

namespace giada::v
{
geCpuLoad::geCpuLoad()
: geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN)
, m_counter(0)
{
	m_text  = new geBox();
	m_meter = new geProgress();

	m_text->align(FL_ALIGN_LEFT);
	m_text->labelcolor(G_COLOR_GREY_4);

	addWidget(m_text, 70);
	addWidget(m_meter);
	end();

	refresh();
}

/* -------------------------------------------------------------------------- */

void geCpuLoad::refresh()
{
	if (++m_counter % (G_GUI_FPS / 2) != 0)
		return;
	m_counter = 0;

	const double load = c::main::getCpuLoad();

	m_text->setLabel(fmt::format("CPU: {:.1f}%", load));
	m_meter->value(load);

	redraw();
}
} // namespace giada::v
