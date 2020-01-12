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
#include "core/channels/sampleChannel.h"
#include "core/model/model.h"
#include "core/const.h"
#include "utils/gui.h"
#include "utils/string.h"
#include "glue/sampleEditor.h"
#include "gui/dialogs/warnings.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/button.h"
#include "shiftTool.h"


namespace giada {
namespace v 
{
geShiftTool::geShiftTool(ID channelId, ID waveId, int x, int y)
: Fl_Pack    (x, y, 300, G_GUI_UNIT),
  m_channelId(channelId),
  m_waveId   (waveId)
{
	type(Fl_Pack::HORIZONTAL);
	spacing(G_GUI_INNER_MARGIN);

	begin();
		m_label = new geBox   (0, 0, u::gui::getStringWidth("Shift"), G_GUI_UNIT, "Shift", FL_ALIGN_RIGHT);
		m_shift = new geInput (0, 0, 70, G_GUI_UNIT);
		m_reset = new geButton(0, 0, 70, G_GUI_UNIT, "Reset");
	end();

	m_shift->type(FL_INT_INPUT);
	m_shift->when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY); // on focus lost or enter key
	m_shift->callback(cb_setShift, (void*)this);

	m_reset->callback(cb_reset, (void*)this);
}


/* -------------------------------------------------------------------------- */


void geShiftTool::cb_setShift(Fl_Widget* w, void* p) { ((geShiftTool*)p)->cb_setShift(); }
void geShiftTool::cb_reset(Fl_Widget* w, void* p) { ((geShiftTool*)p)->cb_reset(); }


/* -------------------------------------------------------------------------- */


void geShiftTool::cb_setShift()
{
	shift(atoi(m_shift->value()));
}


/* -------------------------------------------------------------------------- */


void geShiftTool::cb_reset()
{
	shift(0);
}


/* -------------------------------------------------------------------------- */


void geShiftTool::rebuild()
{
	m::model::onGet(m::model::channels, m_channelId, [&](m::Channel& c)
	{
		m_shift->value(std::to_string(static_cast<m::SampleChannel&>(c).shift).c_str());
	});
}


/* -------------------------------------------------------------------------- */


void geShiftTool::shift(int f)
{
	c::sampleEditor::shift(m_channelId, m_waveId, f);
}
}} // giada::v::
