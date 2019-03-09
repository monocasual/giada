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


#include <cassert>
#include <FL/Fl.H>
#include "core/channels/sampleChannel.h"
#include "core/wave.h"
#include "glue/channel.h"
#include "glue/sampleEditor.h"
#include "utils/gui.h"
#include "utils/string.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/button.h"
#include "waveTools.h"
#include "rangeTool.h"


namespace giada {
namespace v 
{
geRangeTool::geRangeTool(int x, int y)
: Fl_Pack(x, y, 280, G_GUI_UNIT)
{
	type(Fl_Pack::HORIZONTAL);
	spacing(G_GUI_INNER_MARGIN);

	begin();
		m_label = new geBox   (0, 0, u::gui::getStringWidth("Range"), G_GUI_UNIT, "Range", FL_ALIGN_RIGHT);
		m_begin = new geInput (0, 0, 70, G_GUI_UNIT);
		m_end   = new geInput (0, 0, 70, G_GUI_UNIT);
		m_reset = new geButton(0, 0, 70, G_GUI_UNIT, "Reset");
	end();

	m_begin->type(FL_INT_INPUT);
	m_begin->when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY); // on focus lost or enter key
	m_begin->callback(cb_setChanPos, this);
	
	m_end->type(FL_INT_INPUT);
	m_end->when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY); // on focus lost or enter key
	m_end->callback(cb_setChanPos, this);

	m_reset->callback(cb_resetStartEnd, this);
}


/* -------------------------------------------------------------------------- */


void geRangeTool::rebuild()
{
	const m::SampleChannel* ch = static_cast<gdSampleEditor*>(window())->ch;

	m_begin->value(u::string::iToString(ch->getBegin()).c_str());
	m_end->value(u::string::iToString(ch->getEnd()).c_str());
}


/* -------------------------------------------------------------------------- */


void geRangeTool::cb_setChanPos   (Fl_Widget* w, void* p) { ((geRangeTool*)p)->cb_setChanPos(); }
void geRangeTool::cb_resetStartEnd(Fl_Widget* w, void* p) { ((geRangeTool*)p)->cb_resetStartEnd(); }


/* -------------------------------------------------------------------------- */


void geRangeTool::cb_setChanPos()
{
	const m::SampleChannel* ch = static_cast<gdSampleEditor*>(window())->ch;

	c::sampleEditor::setBeginEnd(ch->id, atoi(m_begin->value()), atoi(m_end->value()));
}


/* -------------------------------------------------------------------------- */


void geRangeTool::cb_resetStartEnd()
{
	const m::SampleChannel* ch = static_cast<gdSampleEditor*>(window())->ch;

	c::sampleEditor::setBeginEnd(ch->id, 0, ch->wave->getSize() - 1);
}

}} // giada::v::