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


#include <cmath>
#include <cstdlib>
#include <FL/Fl_Pack.H>
#include "core/channels/sampleChannel.h"
#include "core/const.h"
#include "core/model/model.h"
#include "glue/channel.h"
#include "utils/gui.h"
#include "utils/math.h"
#include "utils/string.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/input.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "volumeTool.h"


namespace giada {
namespace v 
{
geVolumeTool::geVolumeTool(ID channelId, int X, int Y)
: Fl_Pack    (X, Y, 150, G_GUI_UNIT),
  m_channelId(channelId)
{
	type(Fl_Pack::HORIZONTAL);
	spacing(G_GUI_INNER_MARGIN);

	begin();
		label = new geBox  (0, 0, u::gui::getStringWidth("Volume"), G_GUI_UNIT, "Volume", FL_ALIGN_RIGHT);
		dial  = new geDial (0, 0, G_GUI_UNIT, G_GUI_UNIT);
		input = new geInput(0, 0, 70, G_GUI_UNIT);
	end();

	dial->range(0.0f, 1.0f);
	dial->callback(cb_setVolume, (void*)this);

	input->callback(cb_setVolumeNum, (void*)this);
}


/* -------------------------------------------------------------------------- */


void geVolumeTool::rebuild()
{
	float volume;
	m::model::onGet(m::model::channels, m_channelId, [&](m::Channel& c) { volume = c.volume; });

	std::string tmp = "-inf";
	float dB = u::math::linearToDB(volume);
	if (dB > -INFINITY) 
		tmp = u::string::fToString(dB, 2);  // 2 digits
	input->value(tmp.c_str());
	dial->value(volume);
}


/* -------------------------------------------------------------------------- */


void geVolumeTool::cb_setVolume   (Fl_Widget* w, void* p) { ((geVolumeTool*)p)->cb_setVolume(); }
void geVolumeTool::cb_setVolumeNum(Fl_Widget* w, void* p) { ((geVolumeTool*)p)->cb_setVolumeNum(); }


/* -------------------------------------------------------------------------- */


void geVolumeTool::cb_setVolume()
{
	c::channel::setVolume(m_channelId, dial->value(), false, true);
}


/* -------------------------------------------------------------------------- */


void geVolumeTool::cb_setVolumeNum()
{
	float value = pow(10, (atof(input->value()) / 20)); // linear = 10^(dB/20)
	c::channel::setVolume(m_channelId, value, false, true);
}

}} // giada::v::
