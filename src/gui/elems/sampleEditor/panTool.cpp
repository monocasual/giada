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
#include "core/channels/sampleChannel.h"
#include "core/model/model.h"
#include "core/const.h"
#include "core/waveFx.h"  
#include "glue/channel.h"
#include "utils/gui.h"
#include "utils/math.h"
#include "utils/string.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/button.h"
#include "waveTools.h"
#include "panTool.h"


namespace giada {
namespace v 
{
gePanTool::gePanTool(ID channelId, int x, int y)
: Fl_Pack    (x, y, 200, G_GUI_UNIT),
  m_channelId(channelId)
{
	type(Fl_Pack::HORIZONTAL);
	spacing(G_GUI_INNER_MARGIN);

	begin();
		label = new geBox   (0, 0, u::gui::getStringWidth("Pan"), G_GUI_UNIT, "Pan", FL_ALIGN_RIGHT);
		dial  = new geDial  (0, 0, G_GUI_UNIT, G_GUI_UNIT);
		input = new geInput (0, 0, 70, G_GUI_UNIT);
		reset = new geButton(0, 0, 70, G_GUI_UNIT, "Reset");
	end();

	dial->range(0.0f, 1.0f);
	dial->callback(cb_panning, (void*)this);

	input->align(FL_ALIGN_RIGHT);
	input->readonly(1);
	input->cursor_color(FL_WHITE);

	reset->callback(cb_panReset, (void*)this);
}


/* -------------------------------------------------------------------------- */


void gePanTool::rebuild()
{
	float p;
	m::model::onGet(m::model::channels, m_channelId, [&](m::Channel& c)
	{
		p = static_cast<m::SampleChannel&>(c).getPan();
	});

	dial->value(p);

	if (p < 0.5f) {
		std::string tmp = u::string::iToString((int) ((-p * 200.0f) + 100.0f)) + " L";
		input->value(tmp.c_str());
	}
	else 
	if (p == 0.5)
		input->value("C");
	else {
		std::string tmp = u::string::iToString((int) ((p * 200.0f) - 100.0f)) + " R";
		input->value(tmp.c_str());
	}
}


/* -------------------------------------------------------------------------- */


void gePanTool::cb_panning (Fl_Widget* w, void* p) { ((gePanTool*)p)->cb_panning(); }
void gePanTool::cb_panReset(Fl_Widget* w, void* p) { ((gePanTool*)p)->cb_panReset(); }



/* -------------------------------------------------------------------------- */


void gePanTool::cb_panning()
{
	c::channel::setPan(m_channelId, dial->value());
}


/* -------------------------------------------------------------------------- */


void gePanTool::cb_panReset()
{
	c::channel::setPan(m_channelId, 0.5f);
}

}} // giada::v::
