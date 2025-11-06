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

#include "src/gui/elems/mainWindow/scenes.h"
#include "src/glue/main.h"
#include "src/gui/const.h"
#include "src/gui/elems/basics/flex.h"
#include "src/gui/elems/playButton.h"
#include "src/utils/vector.h"
#include <fmt/core.h>

namespace giada::v
{
geScenes::geScenes()
: geFlex(Direction::VERTICAL, G_GUI_INNER_MARGIN)
{
	geFlex* row1 = new geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN);
	{
		row1->addWidget(makeButton(0), G_GUI_UNIT);
		row1->addWidget(makeButton(1), G_GUI_UNIT);
		row1->addWidget(makeButton(2), G_GUI_UNIT);
		row1->addWidget(makeButton(3), G_GUI_UNIT);
		row1->end();
	}

	geFlex* row2 = new geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN);
	{
		row2->addWidget(makeButton(4), G_GUI_UNIT);
		row2->addWidget(makeButton(5), G_GUI_UNIT);
		row2->addWidget(makeButton(6), G_GUI_UNIT);
		row2->addWidget(makeButton(7), G_GUI_UNIT);
		row2->end();
	}

	addWidget(row1);
	addWidget(row2);
}

/* -------------------------------------------------------------------------- */

void geScenes::refresh()
{
	const c::main::Scenes scenes = c::main::getScenes();

	for (const std::size_t i : u::vector::range(m_buttons.size()))
	{
		if (i == scenes.currentScene)
			m_buttons[i]->setPlayState();
		else
			m_buttons[i]->setDefaultState();
		m_buttons[i]->redraw();
	}
}

/* -------------------------------------------------------------------------- */

gePlayButton* geScenes::makeButton(std::size_t scene)
{
	m_buttons[scene] = new gePlayButton(fmt::format("{}", scene + 1));
	m_buttons[scene]->setPadding(0);
	m_buttons[scene]->onClick = [scene]()
	{ c::main::setScene(scene); };

	return m_buttons[scene];
}
} // namespace giada::v
