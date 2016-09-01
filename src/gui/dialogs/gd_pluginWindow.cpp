/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_pluginWindow
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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


#include <FL/Fl_Scroll.H>
#include "../../utils/gui.h"
#include "../../core/plugin.h"
#include "../elems/ge_mixed.h"
#include "gd_pluginWindow.h"


using std::string;


Parameter::Parameter(int paramIndex, Plugin *p, int X, int Y, int W)
	: Fl_Group(X, Y, W-24, 20), paramIndex(paramIndex), pPlugin(p)
{
	begin();

		label = new gBox(x(), y(), 60, 20);
		label->copy_label(pPlugin->getParameterName(paramIndex).toRawUTF8());
		label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

		slider = new gSlider(label->x()+label->w()+8, y(), W-200, 20);
		slider->value(pPlugin->getParameter(paramIndex));
		slider->callback(cb_setValue, (void *)this);

		value = new gBox(slider->x()+slider->w()+8, y(), 100, 20);
		value->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
		value->box(G_BOX);
		updateValue();

		resizable(slider);

	end();
}


/* -------------------------------------------------------------------------- */


void Parameter::cb_setValue(Fl_Widget *v, void *p)  { ((Parameter*)p)->__cb_setValue(); }


/* -------------------------------------------------------------------------- */


void Parameter::__cb_setValue()
{
	pPlugin->setParameter(paramIndex, slider->value());
	updateValue();
	value->redraw();
}


/* -------------------------------------------------------------------------- */


void Parameter::updateValue()
{
	string v = pPlugin->getParameterText(paramIndex).toStdString() + " " +
			pPlugin->getParameterLabel(paramIndex).toStdString();
	value->copy_label(v.c_str());
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


gdPluginWindow::gdPluginWindow(Plugin *p)
 : gWindow(400, 156), pPlugin(p) // 350
{
	set_non_modal();

	gLiquidScroll *list = new gLiquidScroll(8, 8, w()-16, h()-16);
	list->type(Fl_Scroll::VERTICAL_ALWAYS);
	list->begin();

	int numParams = pPlugin->getNumParameters();
	for (int i=0; i<numParams; i++)
		new Parameter(i, pPlugin, list->x(), list->y()+(i*24), list->w());
	list->end();

	end();

	label(pPlugin->getName().toRawUTF8());

	size_range(400, (24*1)+12);
	resizable(list);

	gu_setFavicon(this);
	show();
}


#endif // #ifdef WITH_VST
