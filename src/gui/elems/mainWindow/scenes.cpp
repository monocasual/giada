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
#include "src/gui/elems/basics/textButton.h"

namespace giada::v
{
geScenes::geScenes()
: geFlex(Direction::VERTICAL, G_GUI_OUTER_MARGIN)
{
	geFlex* row1 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
	{
		m_buttons[0] = new geTextButton("A");
		m_buttons[1] = new geTextButton("B");
		m_buttons[2] = new geTextButton("C");
		m_buttons[3] = new geTextButton("D");
		row1->addWidget(m_buttons[0], G_GUI_UNIT);
		row1->addWidget(m_buttons[1], G_GUI_UNIT);
		row1->addWidget(m_buttons[2], G_GUI_UNIT);
		row1->addWidget(m_buttons[3], G_GUI_UNIT);
		row1->end();
	}

	geFlex* row2 = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
	{
		m_buttons[4] = new geTextButton("E");
		m_buttons[5] = new geTextButton("F");
		m_buttons[6] = new geTextButton("G");
		m_buttons[7] = new geTextButton("H");

		row2->addWidget(m_buttons[4], G_GUI_UNIT);
		row2->addWidget(m_buttons[5], G_GUI_UNIT);
		row2->addWidget(m_buttons[6], G_GUI_UNIT);
		row2->addWidget(m_buttons[7], G_GUI_UNIT);
		row2->end();
	}

	addWidget(row1);
	addWidget(row2);

	for (std::size_t i = 0; i < m_buttons.size(); i++)
	{
		m_buttons[i]->onClick = [i]()
		{ c::main::setScene(i); };
	}
}
} // namespace giada::v
