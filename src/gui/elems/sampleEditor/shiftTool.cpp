/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#include <cassert>
#include <cstdlib>
#include "core/model/model.h"
#include "core/const.h"
#include "utils/gui.h"
#include "utils/string.h"
#include "glue/sampleEditor.h"
#include "gui/dialogs/warnings.h"
#include "gui/dialogs/sampleEditor.h"
#include "shiftTool.h"


namespace giada {
namespace v 
{
geShiftTool::geShiftTool(const c::sampleEditor::Data& d, int x, int y)
: gePack (x, y, Direction::HORIZONTAL)
, m_data (nullptr)
, m_label(0, 0, 60, G_GUI_UNIT, "Shift", FL_ALIGN_LEFT)
, m_shift(0, 0, 70, G_GUI_UNIT)
, m_reset(0, 0, 70, G_GUI_UNIT, "Reset")
{
	add(&m_label);
	add(&m_shift);
	add(&m_reset);

	m_shift.type(FL_INT_INPUT);
	m_shift.when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY); // on focus lost or enter key
	m_shift.callback(cb_setShift, (void*)this);

	m_reset.callback(cb_reset, (void*)this);

	rebuild(d);
}


/* -------------------------------------------------------------------------- */


void geShiftTool::cb_setShift(Fl_Widget* /*w*/, void* p) { ((geShiftTool*)p)->cb_setShift(); }
void geShiftTool::cb_reset(Fl_Widget* /*w*/, void* p) { ((geShiftTool*)p)->cb_reset(); }


/* -------------------------------------------------------------------------- */


void geShiftTool::cb_setShift()
{
	shift(atoi(m_shift.value()));
}


/* -------------------------------------------------------------------------- */


void geShiftTool::cb_reset()
{
	shift(0);
}


/* -------------------------------------------------------------------------- */


void geShiftTool::rebuild(const c::sampleEditor::Data& d)
{
	m_data = &d;
	update(m_data->shift);
}


/* -------------------------------------------------------------------------- */


void geShiftTool::update(Frame shift)
{
	m_shift.value(std::to_string(shift).c_str());
}


/* -------------------------------------------------------------------------- */


void geShiftTool::shift(int f)
{
	c::sampleEditor::shift(m_data->channelId, m_data->waveId, f);
}
}} // giada::v::
