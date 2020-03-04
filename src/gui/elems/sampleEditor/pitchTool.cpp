
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


#include <FL/Fl.H>
#include "core/model/model.h"
#include "core/const.h"
#include "core/graphics.h"  
#include "core/clock.h"
#include "glue/events.h"
#include "utils/gui.h"
#include "utils/string.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/button.h"
#include "pitchTool.h"


namespace giada {
namespace v 
{
gePitchTool::gePitchTool(const c::sampleEditor::Data& d, int x, int y)
: Fl_Pack(x, y, 600, G_GUI_UNIT)
, m_data (nullptr)
{
	type(Fl_Pack::HORIZONTAL);
	spacing(G_GUI_INNER_MARGIN);
	
	begin();
		label       = new geBox   (0, 0, u::gui::getStringWidth("Pitch"), G_GUI_UNIT, "Pitch", FL_ALIGN_RIGHT);
		dial        = new geDial  (0, 0, G_GUI_UNIT, G_GUI_UNIT);
		input       = new geInput (0, 0, 70, G_GUI_UNIT);
		pitchToBar  = new geButton(0, 0, 70, G_GUI_UNIT, "To bar");
		pitchToSong = new geButton(0, 0, 70, G_GUI_UNIT, "To song");
		pitchHalf   = new geButton(0, 0, G_GUI_UNIT, G_GUI_UNIT, "", divideOff_xpm, divideOn_xpm);
		pitchDouble = new geButton(0, 0, G_GUI_UNIT, G_GUI_UNIT, "", multiplyOff_xpm, multiplyOn_xpm);
		pitchReset  = new geButton(0, 0, 70, G_GUI_UNIT, "Reset");
	end();

	dial->range(0.01f, 4.0f);
	dial->callback(cb_setPitch, (void*)this);
	dial->when(FL_WHEN_RELEASE);

	input->align(FL_ALIGN_RIGHT);
	input->callback(cb_setPitchNum, (void*)this);
	input->when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY);

	pitchToBar->callback(cb_setPitchToBar, (void*)this);
	pitchToSong->callback(cb_setPitchToSong, (void*)this);
	pitchHalf->callback(cb_setPitchHalf, (void*)this);
	pitchDouble->callback(cb_setPitchDouble, (void*)this);
	pitchReset->callback(cb_resetPitch, (void*)this);

	rebuild(d);
}


/* -------------------------------------------------------------------------- */


void gePitchTool::rebuild(const c::sampleEditor::Data& d)
{
	m_data = &d;
	update(m_data->pitch, /*isDial=*/false);
}


/* -------------------------------------------------------------------------- */


void gePitchTool::update(float v, bool isDial)
{
	input->value(u::string::fToString(v, 4).c_str()); // 4 digits
	if (!isDial)
		dial->value(v);
}


/* -------------------------------------------------------------------------- */


void gePitchTool::cb_setPitch      (Fl_Widget* w, void* p) { ((gePitchTool*)p)->cb_setPitch(); }
void gePitchTool::cb_setPitchToBar (Fl_Widget* w, void* p) { ((gePitchTool*)p)->cb_setPitchToBar(); }
void gePitchTool::cb_setPitchToSong(Fl_Widget* w, void* p) { ((gePitchTool*)p)->cb_setPitchToSong(); }
void gePitchTool::cb_setPitchHalf  (Fl_Widget* w, void* p) { ((gePitchTool*)p)->cb_setPitchHalf(); }
void gePitchTool::cb_setPitchDouble(Fl_Widget* w, void* p) { ((gePitchTool*)p)->cb_setPitchDouble(); }
void gePitchTool::cb_resetPitch    (Fl_Widget* w, void* p) { ((gePitchTool*)p)->cb_resetPitch(); }
void gePitchTool::cb_setPitchNum   (Fl_Widget* w, void* p) { ((gePitchTool*)p)->cb_setPitchNum(); }


/* -------------------------------------------------------------------------- */


void gePitchTool::cb_setPitch()
{
	c::events::setChannelPitch(m_data->channelId, dial->value(), Thread::MAIN);
}


/* -------------------------------------------------------------------------- */


void gePitchTool::cb_setPitchNum()
{
	c::events::setChannelPitch(m_data->channelId, atof(input->value()), Thread::MAIN);	
}


/* -------------------------------------------------------------------------- */


void gePitchTool::cb_setPitchHalf()
{
	c::events::setChannelPitch(m_data->channelId, dial->value() / 2, Thread::MAIN);	
}


/* -------------------------------------------------------------------------- */


void gePitchTool::cb_setPitchDouble()
{
	c::events::setChannelPitch(m_data->channelId, dial->value() * 2, Thread::MAIN);	
}


/* -------------------------------------------------------------------------- */


void gePitchTool::cb_setPitchToBar()
{
	c::events::setChannelPitch(m_data->channelId, m_data->end / (float) m::clock::getFramesInBar(), 
		Thread::MAIN);	
}


/* -------------------------------------------------------------------------- */


void gePitchTool::cb_setPitchToSong()
{
	c::events::setChannelPitch(m_data->channelId, m_data->end / (float) m::clock::getFramesInLoop(), 
		Thread::MAIN);	
}


/* -------------------------------------------------------------------------- */


void gePitchTool::cb_resetPitch()
{
	c::events::setChannelPitch(m_data->channelId, G_DEFAULT_PITCH, Thread::MAIN);	
}
}} // giada::v::
