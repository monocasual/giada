/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#include "../../../core/plugin.h"
#include "../../../core/const.h"
#include "../../../glue/plugin.h"
#include "../basics/boxtypes.h"
#include "../basics/box.h"
#include "../basics/slider.h"
#include "pluginParameter.h"


using std::string;
using namespace giada::c;


gePluginParameter::gePluginParameter(int paramIndex, Plugin* p, int X, int Y, 
	int W, int labelWidth)
	: Fl_Group    (X, Y, W, G_GUI_UNIT), 
	  m_paramIndex(paramIndex), 
	  m_plugin    (p)
{
	begin();

		m_label = new geBox(x(), y(), labelWidth, G_GUI_UNIT);
		m_label->copy_label(m_plugin->getParameterName(m_paramIndex).c_str());
		m_label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

		m_slider = new geSlider(m_label->x()+m_label->w()+G_GUI_OUTER_MARGIN, y(), 
			w()-(m_label->x()+m_label->w()+G_GUI_OUTER_MARGIN)-VALUE_WIDTH, G_GUI_UNIT);
		m_slider->value(m_plugin->getParameter(m_paramIndex));
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
	plugin::setParameter(m_plugin, m_paramIndex, m_slider->value());
}


/* -------------------------------------------------------------------------- */


void gePluginParameter::update(bool changeSlider)
{
	string v = m_plugin->getParameterText(m_paramIndex) + " " +
			m_plugin->getParameterLabel(m_paramIndex);
	m_value->copy_label(v.c_str());
	if (changeSlider)
		m_slider->value(m_plugin->getParameter(m_paramIndex));
}


#endif // #ifdef WITH_VST
