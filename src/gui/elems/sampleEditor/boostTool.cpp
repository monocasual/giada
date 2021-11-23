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

#include "boostTool.h"
#include "core/const.h"
#include "core/waveFx.h"
#include "glue/channel.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/input.h"
#include "utils/gui.h"
#include "utils/math.h"
#include "utils/string.h"
#include "waveTools.h"
#include <FL/Fl.H>

namespace giada
{
namespace v
{
geBoostTool::geBoostTool(int X, int Y)
: Fl_Pack(X, Y, 220, G_GUI_UNIT)
{
	type(Fl_Pack::HORIZONTAL);
	spacing(G_GUI_INNER_MARGIN);

	begin();
	label     = new geBox(0, 0, u::gui::getStringRect("Boost").w, G_GUI_UNIT, "Boost", FL_ALIGN_RIGHT);
	dial      = new geDial(0, 0, G_GUI_UNIT, G_GUI_UNIT);
	input     = new geInput(0, 0, 70, G_GUI_UNIT);
	normalize = new geButton(0, 0, 70, G_GUI_UNIT, "Normalize");
	end();

	dial->range(1.0f, 10.0f);
	dial->callback(cb_setBoost, (void*)this);
	dial->when(FL_WHEN_CHANGED | FL_WHEN_RELEASE);

	input->callback(cb_setBoostNum, (void*)this);

	normalize->callback(cb_normalize, (void*)this);
}

/* -------------------------------------------------------------------------- */

void geBoostTool::rebuild()
{
	/*
	const m::SampleChannel* ch = static_cast<gdSampleEditor*>(window())->ch;

	input->value(u::string::fToString(u::math::linearToDB(ch->getBoost()), 2).c_str());  // 2 digits
	// A dial greater than it's max value goes crazy
	dial->value(ch->getBoost() <= 10.0f ? ch->getBoost() : 10.0f);*/
}

/* -------------------------------------------------------------------------- */

void geBoostTool::cb_setBoost(Fl_Widget* /*w*/, void* p) { ((geBoostTool*)p)->cb_setBoost(); }
void geBoostTool::cb_setBoostNum(Fl_Widget* /*w*/, void* p) { ((geBoostTool*)p)->cb_setBoostNum(); }
void geBoostTool::cb_normalize(Fl_Widget* /*w*/, void* p) { ((geBoostTool*)p)->cb_normalize(); }

/* -------------------------------------------------------------------------- */

void geBoostTool::cb_setBoost()
{
	/*const m::SampleChannel* ch = static_cast<gdSampleEditor*>(window())->ch;

	c::channel::setBoost(ch->id, dial->value());*/
}

/* -------------------------------------------------------------------------- */

void geBoostTool::cb_setBoostNum()
{
	/*const m::SampleChannel* ch = static_cast<gdSampleEditor*>(window())->ch;

	c::channel::setBoost(ch->id, u::math::dBtoLinear(atof(input->value())));*/
}

/* -------------------------------------------------------------------------- */

void geBoostTool::cb_normalize()
{
}

} // namespace v
} // namespace giada
