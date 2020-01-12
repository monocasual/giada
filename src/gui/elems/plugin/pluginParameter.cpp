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


#ifdef WITH_VST


#include "core/model/model.h"
#include "core/plugin.h"
#include "core/const.h"
#include "glue/plugin.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/slider.h"
#include "pluginParameter.h"


namespace giada {
namespace v
{
gePluginParameter::gePluginParameter(int paramIndex, ID pluginId, 
	int X, int Y, int W, int labelWidth)
: Fl_Group    (X, Y, W, G_GUI_UNIT), 
  m_pluginId  (pluginId),
  m_paramIndex(paramIndex)
{
	m::model::PluginsLock l(m::model::plugins);
	const m::Plugin& p = m::model::get(m::model::plugins, m_pluginId);

	begin();

		const int VALUE_WIDTH = 100;

		m_label = new geBox(x(), y(), labelWidth, G_GUI_UNIT);
		m_label->copy_label(p.getParameterName(m_paramIndex).c_str());
		m_label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

		m_slider = new geSlider(m_label->x()+m_label->w()+G_GUI_OUTER_MARGIN, y(), 
			w()-(m_label->x()+m_label->w()+G_GUI_OUTER_MARGIN)-VALUE_WIDTH, G_GUI_UNIT);
		m_slider->value(p.getParameter(m_paramIndex));
		m_slider->callback(cb_setValue, (void*)this);

		m_value = new geBox(m_slider->x()+m_slider->w()+G_GUI_OUTER_MARGIN, y(), VALUE_WIDTH, G_GUI_UNIT);
		m_value->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
		m_value->box(G_CUSTOM_BORDER_BOX);

	end();
	resizable(m_slider);
	update(false);
}


/* -------------------------------------------------------------------------- */


void gePluginParameter::cb_setValue(Fl_Widget* v, void* p)  { ((gePluginParameter*)p)->cb_setValue(); }


/* -------------------------------------------------------------------------- */


void gePluginParameter::cb_setValue()
{
	c::plugin::setParameter(m_pluginId, m_paramIndex, m_slider->value(), 
		/*gui=*/true);
}


/* -------------------------------------------------------------------------- */


void gePluginParameter::update(bool changeSlider)
{
	m::model::PluginsLock l(m::model::plugins);
	const m::Plugin& p = m::model::get(m::model::plugins, m_pluginId);

	std::string v = p.getParameterText(m_paramIndex) + " " +
	                p.getParameterLabel(m_paramIndex);

	m_value->copy_label(v.c_str());

	if (changeSlider)
		m_slider->value(p.getParameter(m_paramIndex));
}
}} // giada::v::


#endif // #ifdef WITH_VST
