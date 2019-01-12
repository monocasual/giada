/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#include <cstdlib>
#include "../../../core/const.h"
#include "../../../core/sampleChannel.h"
#include "../../../utils/gui.h"
#include "../../../utils/string.h"
#include "../../../glue/sampleEditor.h"
#include "../../dialogs/warnings.h"
#include "../basics/input.h"
#include "../basics/box.h"
#include "../basics/button.h"
#include "shiftTool.h"


using namespace giada;


geShiftTool::geShiftTool(int x, int y, giada::m::SampleChannel* ch)
	: Fl_Group(x, y, 300, G_GUI_UNIT),
		m_ch    (ch)
{
	begin();
		m_label = new geBox(x, y, gu_getStringWidth("Shift"), G_GUI_UNIT, "Shift", FL_ALIGN_RIGHT);
		m_shift = new geInput(m_label->x()+m_label->w()+G_GUI_INNER_MARGIN, y, 70, G_GUI_UNIT);
		m_reset = new geButton(m_shift->x()+m_shift->w()+G_GUI_INNER_MARGIN, y, 70, G_GUI_UNIT, "Reset");
	end();

	m_shift->type(FL_INT_INPUT);
	m_shift->when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY); // on focus lost or enter key
	m_shift->value(u::string::iToString(ch->shift).c_str());
	m_shift->callback(cb_setShift, (void*)this);

	m_reset->callback(cb_reset, (void*)this);

	refresh();
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


void geShiftTool::refresh()
{
	m_shift->value(u::string::iToString(m_ch->shift).c_str());
}


/* -------------------------------------------------------------------------- */


void geShiftTool::shift(int f)
{
	if (m_ch->isPlaying())
		gdAlert("Can't shift sample while playing.");
	else
		c::sampleEditor::shift(m_ch, f);	
}
