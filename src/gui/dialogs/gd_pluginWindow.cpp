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


#include <FL/fl_draw.H>
#include <FL/Fl_Scroll.H>
#include "../../utils/gui.h"
#include "../../core/plugin.h"
#include "../../core/const.h"
#include "../elems/basics/boxtypes.h"
#include "../elems/basics/box.h"
#include "../elems/basics/liquidScroll.h"
#include "../elems/basics/slider.h"
#include "gd_pluginWindow.h"


using std::string;


Parameter::Parameter(int paramIndex, Plugin* p, int X, int Y, int W, 
	int labelWidth)
	: Fl_Group(X, Y, W, G_GUI_UNIT), paramIndex(paramIndex), pPlugin(p)
{
	begin();

		label = new geBox(x(), y(), labelWidth, G_GUI_UNIT);
		label->copy_label(pPlugin->getParameterName(paramIndex).c_str());
		label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

		slider = new geSlider(label->x()+label->w()+G_GUI_OUTER_MARGIN, y(), 
			w()-(label->x()+label->w()+G_GUI_OUTER_MARGIN)-VALUE_WIDTH, G_GUI_UNIT);
		slider->value(pPlugin->getParameter(paramIndex));
		slider->callback(cb_setValue, (void*)this);

		value = new geBox(slider->x()+slider->w()+G_GUI_OUTER_MARGIN, y(), VALUE_WIDTH, G_GUI_UNIT);
		value->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
		value->box(G_CUSTOM_BORDER_BOX);

	end();
	resizable(slider);
	updateValue();
}


/* -------------------------------------------------------------------------- */


void Parameter::cb_setValue(Fl_Widget* v, void* p)  { ((Parameter*)p)->cb_setValue(); }


/* -------------------------------------------------------------------------- */


void Parameter::cb_setValue()
{
	pPlugin->setParameter(paramIndex, slider->value());
	updateValue();
	value->redraw();
}


/* -------------------------------------------------------------------------- */


void Parameter::updateValue()
{
	string v = pPlugin->getParameterText(paramIndex) + " " +
			pPlugin->getParameterLabel(paramIndex);
	value->copy_label(v.c_str());
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


gdPluginWindow::gdPluginWindow(Plugin* p)
 : gdWindow(450, 156), pPlugin(p)
{
	set_non_modal();

	geLiquidScroll* list = new geLiquidScroll(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN, 
		w()-(G_GUI_OUTER_MARGIN*2), h()-(G_GUI_OUTER_MARGIN*2));

	list->type(Fl_Scroll::VERTICAL_ALWAYS);
	list->begin();
		int labelWidth = getLabelWidth();
		int numParams = pPlugin->getNumParameters();
		for (int i=0; i<numParams; i++) {
			int py = list->y() + (i * (G_GUI_UNIT + G_GUI_INNER_MARGIN));
			int pw = list->w() - list->scrollbar_size() - (G_GUI_OUTER_MARGIN*3);
			new Parameter(i, pPlugin, list->x(), py, pw, labelWidth);
		}
	list->end();

	end();

	label(pPlugin->getName().c_str());

	size_range(450, (G_GUI_UNIT + (G_GUI_OUTER_MARGIN*2)));
	resizable(list);

	gu_setFavicon(this);
	show();
}


/* -------------------------------------------------------------------------- */


int gdPluginWindow::getLabelWidth() const
{
	int width = 0;
	int numParams = pPlugin->getNumParameters();
	for (int i=0; i<numParams; i++) {
		int wl = 0, hl = 0;   
		fl_measure(pPlugin->getParameterName(i).c_str(), wl, hl);
		if (wl > width)
			width = wl;
	}
	return width;
}


#endif // #ifdef WITH_VST
