/* -----------------------------------------------------------------------------
*
* Giada - Your Hardcore Loopmachine
*
* ------------------------------------------------------------------------------
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
* --------------------------------------------------------------------------- */

#include "gui/elems/actionEditor/gridTool.h"
#include "core/conf.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/choice.h"
#include "gui/ui.h"
#include "utils/math.h"
#include <FL/Fl_Double_Window.H>

extern giada::v::Ui g_ui;

namespace giada::v
{
geGridTool::geGridTool(Pixel x, Pixel y, m::Conf& c)
: Fl_Group(x, y, 80, 20)
, m_conf(c)
{
	gridType = new geChoice(x, y, 40, 20);
	gridType->addItem("1");
	gridType->addItem("2");
	gridType->addItem("3");
	gridType->addItem("4");
	gridType->addItem("6");
	gridType->addItem("8");
	gridType->addItem("16");
	gridType->addItem("32");
	gridType->showItem(0);
	gridType->onChange = [this](ID) {
		window()->redraw();
	};

	active = new geCheck(gridType->x() + gridType->w() + 4, y, 20, 20);

	gridType->showItem(m_conf.actionEditorGridVal);
	active->value(m_conf.actionEditorGridOn);

	end();

	gridType->copy_tooltip(g_ui.getI18Text(LangMap::COMMON_GRIDRES));
	active->copy_tooltip(g_ui.getI18Text(LangMap::COMMON_SNAPTOGRID));
}

/* -------------------------------------------------------------------------- */

geGridTool::~geGridTool()
{
	m_conf.actionEditorGridVal = gridType->getSelectedId();
	m_conf.actionEditorGridOn  = active->value();
}

/* -------------------------------------------------------------------------- */

bool geGridTool::isOn() const
{
	return active->value();
}

/* -------------------------------------------------------------------------- */

int geGridTool::getValue() const
{
	switch (gridType->getSelectedId())
	{
	case 0:
		return 1;
	case 1:
		return 2;
	case 2:
		return 3;
	case 3:
		return 4;
	case 4:
		return 6;
	case 5:
		return 8;
	case 6:
		return 16;
	case 7:
		return 32;
	}
	return 0;
}

/* -------------------------------------------------------------------------- */

Frame geGridTool::getSnapFrame(Frame v, Frame framesInBeat) const
{
	if (!isOn())
		return v;
	return u::math::quantize(v, getCellSize(framesInBeat));
}

/* -------------------------------------------------------------------------- */

Frame geGridTool::getCellSize(Frame framesInBeat) const
{
	return framesInBeat / getValue();
}
} // namespace giada::v