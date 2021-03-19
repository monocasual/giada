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

#ifdef WITH_VST

#include "pluginParameter.h"
#include "core/const.h"
#include "glue/events.h"
#include "glue/plugin.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/slider.h"

namespace giada
{
namespace v
{
gePluginParameter::gePluginParameter(int X, int Y, int W, int labelWidth, const c::plugin::Param p)
: Fl_Group(X, Y, W, G_GUI_UNIT)
, m_param(p)
{
	begin();

	const int VALUE_WIDTH = 100;

	m_label = new geBox(x(), y(), labelWidth, G_GUI_UNIT);
	m_label->copy_label(m_param.name.c_str());

	m_slider = new geSlider(m_label->x() + m_label->w() + G_GUI_OUTER_MARGIN, y(),
	    w() - (m_label->x() + m_label->w() + G_GUI_OUTER_MARGIN) - VALUE_WIDTH, G_GUI_UNIT);
	m_slider->value(m_param.value);
	m_slider->callback(cb_setValue, (void*)this);

	m_value = new geBox(m_slider->x() + m_slider->w() + G_GUI_OUTER_MARGIN, y(), VALUE_WIDTH, G_GUI_UNIT);
	m_value->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
	m_value->box(G_CUSTOM_BORDER_BOX);

	end();

	resizable(m_slider);
	update(m_param, false);
}

/* -------------------------------------------------------------------------- */

void gePluginParameter::cb_setValue(Fl_Widget* /*w*/, void* p) { ((gePluginParameter*)p)->cb_setValue(); }

/* -------------------------------------------------------------------------- */

void gePluginParameter::cb_setValue()
{
	c::events::setPluginParameter(m_param.pluginId, m_param.index,
	    m_slider->value(), /*gui=*/true);
}

/* -------------------------------------------------------------------------- */

void gePluginParameter::update(const c::plugin::Param& p, bool changeSlider)
{
	m_value->copy_label(std::string(p.text + " " + p.label).c_str());
	if (changeSlider)
		m_slider->value(p.value);
}
} // namespace v
} // namespace giada

#endif // #ifdef WITH_VST
