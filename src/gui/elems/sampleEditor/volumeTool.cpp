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


#include <cmath>
#include <cstdlib>
#include <FL/Fl_Pack.H>
#include "core/const.h"
#include "glue/events.h"
#include "utils/gui.h"
#include "utils/math.h"
#include "utils/string.h"
#include "gui/dialogs/sampleEditor.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "volumeTool.h"


namespace giada {
namespace v 
{
geVolumeTool::geVolumeTool(const c::sampleEditor::Data& d, int x, int y)
: gePack (x, y, Direction::HORIZONTAL)
, m_data (nullptr)
, m_label(0, 0, 60, G_GUI_UNIT, "Volume", FL_ALIGN_LEFT)
, m_dial (0, 0, G_GUI_UNIT, G_GUI_UNIT)
, m_input(0, 0, 70, G_GUI_UNIT)
{		
	add(&m_label);
	add(&m_dial);
	add(&m_input);

	m_dial.range(0.0f, 1.0f);
	m_dial.callback(cb_setVolume, (void*)this);

	m_input.callback(cb_setVolumeNum, (void*)this);

	rebuild(d);
}


/* -------------------------------------------------------------------------- */


void geVolumeTool::rebuild(const c::sampleEditor::Data& d)
{
	m_data = &d;
	update(m_data->volume, /*isDial=*/false);
}


/* -------------------------------------------------------------------------- */


void geVolumeTool::update(float v, bool isDial)
{
	std::string tmp = "-inf";
	float dB = u::math::linearToDB(v);
	if (dB > -INFINITY) 
		tmp = u::string::fToString(dB, 2);  // 2 digits
	m_input.value(tmp.c_str());
	if (!isDial)
		m_dial.value(v);
}


/* -------------------------------------------------------------------------- */


void geVolumeTool::cb_setVolume   (Fl_Widget* /*w*/, void* p) { ((geVolumeTool*)p)->cb_setVolume(); }
void geVolumeTool::cb_setVolumeNum(Fl_Widget* /*w*/, void* p) { ((geVolumeTool*)p)->cb_setVolumeNum(); }


/* -------------------------------------------------------------------------- */


void geVolumeTool::cb_setVolume()
{
	c::events::setChannelVolume(m_data->channelId, m_dial.value(), Thread::MAIN);
}


/* -------------------------------------------------------------------------- */


void geVolumeTool::cb_setVolumeNum()
{
	c::events::setChannelVolume(m_data->channelId, u::math::dBtoLinear(atof(m_input.value())), 
		Thread::MAIN);
}
}} // giada::v::
