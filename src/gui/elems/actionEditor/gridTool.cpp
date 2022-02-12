/* -----------------------------------------------------------------------------
*
* Giada - Your Hardcore Loopmachine
*
* ------------------------------------------------------------------------------
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
* --------------------------------------------------------------------------- */

#include "gui/elems/actionEditor/gridTool.h"
#include "core/conf.h"
#include "gui/elems/basics/check.h"
#include "gui/elems/basics/choice.h"
#include "utils/math.h"
#include <FL/Fl_Double_Window.H>

namespace giada::v
{
geGridTool::geGridTool(Pixel x, Pixel y, m::Conf::Data& c)
: Fl_Group(x, y, 80, 20)
, m_conf(c)
{
	gridType = new geChoice(x, y, 40, 20);
	gridType->add("1");
	gridType->add("2");
	gridType->add("3");
	gridType->add("4");
	gridType->add("6");
	gridType->add("8");
	gridType->add("16");
	gridType->add("32");
	gridType->value(0);
	gridType->callback(cb_changeType, (void*)this);

	active = new geCheck(gridType->x() + gridType->w() + 4, y, 20, 20);

	gridType->value(m_conf.actionEditorGridVal);
	active->value(m_conf.actionEditorGridOn);

	end();

	gridType->copy_tooltip("Grid resolution");
	active->copy_tooltip("Snap to grid");
}

/* -------------------------------------------------------------------------- */

geGridTool::~geGridTool()
{
	m_conf.actionEditorGridVal = gridType->value();
	m_conf.actionEditorGridOn  = active->value();
}

/* -------------------------------------------------------------------------- */

void geGridTool::cb_changeType(Fl_Widget* /*w*/, void* p) { ((geGridTool*)p)->cb_changeType(); }

/* -------------------------------------------------------------------------- */

void geGridTool::cb_changeType()
{
	window()->redraw();
}

/* -------------------------------------------------------------------------- */

bool geGridTool::isOn() const
{
	return active->value();
}

/* -------------------------------------------------------------------------- */

int geGridTool::getValue() const
{
	switch (gridType->value())
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