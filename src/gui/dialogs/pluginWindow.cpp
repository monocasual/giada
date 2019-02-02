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


#ifdef WITH_VST


#include <FL/fl_draw.H>
#include "../../utils/gui.h"
#include "../../core/plugin.h"
#include "../../core/const.h"
#include "../elems/basics/liquidScroll.h"
#include "../elems/plugin/pluginParameter.h"
#include "pluginWindow.h"


using namespace giada;


gdPluginWindow::gdPluginWindow(m::Plugin* p)
 : gdWindow(450, 156), m_plugin(p)
{
	set_non_modal();

	m_list = new geLiquidScroll(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN, 
		w()-(G_GUI_OUTER_MARGIN*2), h()-(G_GUI_OUTER_MARGIN*2));

	m_list->type(Fl_Scroll::VERTICAL_ALWAYS);
	m_list->begin();
		int labelWidth = getLabelWidth();
		int numParams = m_plugin->getNumParameters();
		for (int i=0; i<numParams; i++) {
			int py = m_list->y() + (i * (G_GUI_UNIT + G_GUI_INNER_MARGIN));
			int pw = m_list->w() - m_list->scrollbar_size() - (G_GUI_OUTER_MARGIN*3);
			new gePluginParameter(i, m_plugin, m_list->x(), py, pw, labelWidth);
		}
	m_list->end();

	end();

	label(m_plugin->getName().c_str());

	size_range(450, (G_GUI_UNIT + (G_GUI_OUTER_MARGIN*2)));
	resizable(m_list);

	u::gui::setFavicon(this);
	show();
}


/* -------------------------------------------------------------------------- */


void gdPluginWindow::updateParameter(int index, bool changeSlider)
{
	static_cast<gePluginParameter*>(m_list->child(index))->update(changeSlider);
}


void gdPluginWindow::updateParameters(bool changeSlider)
{
	for (int i=0; i<m_plugin->getNumParameters(); i++) {
		static_cast<gePluginParameter*>(m_list->child(i))->update(changeSlider);
	}
}


/* -------------------------------------------------------------------------- */


int gdPluginWindow::getLabelWidth() const
{
	int width = 0;
	int numParams = m_plugin->getNumParameters();
	for (int i=0; i<numParams; i++) {
		int wl = 0, hl = 0;   
		fl_measure(m_plugin->getParameterName(i).c_str(), wl, hl);
		if (wl > width)
			width = wl;
	}
	return width;
}


#endif // #ifdef WITH_VST
