/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_pluginWindow
 *
 * ---------------------------------------------------------------------
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
 * ------------------------------------------------------------------ */


#ifdef WITH_VST


#include <FL/Fl_Scroll.H>
#include "../../utils/gui_utils.h"
#include "../../core/pluginHost_DEPR_.h"
#include "../elems/ge_mixed.h"
#include "gd_pluginWindow.h"



extern PluginHost_DEPR_ G_PluginHost;


Parameter::Parameter(int id, Plugin_DEPR_ *p, int X, int Y, int W)
	: Fl_Group(X,Y,W-24,20), id(id), pPlugin(p)
{
	begin();

		label = new gBox(x(), y(), 60, 20);
		char name[kVstMaxParamStrLen];
		pPlugin->getParamName(id, name);
		label->copy_label(name);
		label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

		slider = new gSlider(label->x()+label->w()+8, y(), W-200, 20);
		slider->value(pPlugin->getParam(id));
		slider->callback(cb_setValue, (void *)this);

		value = new gBox(slider->x()+slider->w()+8, y(), 100, 20);
		char disp[kVstMaxParamStrLen];
		char labl[kVstMaxParamStrLen];
		char str [256];
		pPlugin->getParamDisplay(id, disp);
		pPlugin->getParamLabel(id, labl);
		sprintf(str, "%s %s", disp, labl);
		value->copy_label(str);
		value->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);
		value->box(G_BOX);

		resizable(slider);

	end();
}


/* ------------------------------------------------------------------ */


void Parameter::cb_setValue(Fl_Widget *v, void *p)  { ((Parameter*)p)->__cb_setValue(); }


/* ------------------------------------------------------------------ */


void Parameter::__cb_setValue() {

	pPlugin->setParam(id, slider->value());

	char disp[256];
	char labl[256];
	char str [256];

	pPlugin->getParamDisplay(id, disp);
	pPlugin->getParamLabel(id, labl);
	sprintf(str, "%s %s", disp, labl);

	value->copy_label(str);
	value->redraw();
}


/* ------------------------------------------------------------------ */


gdPluginWindow::gdPluginWindow(Plugin_DEPR_ *pPlugin)
 : gWindow(400, 156), pPlugin(pPlugin) // 350
{
	set_non_modal();

	gLiquidScroll *list = new gLiquidScroll(8, 8, w()-16, h()-16);
	list->type(Fl_Scroll::VERTICAL_ALWAYS);
	list->begin();

	int numParams = pPlugin->getNumParams();
	for (int i=0; i<numParams; i++)
		new Parameter(i, pPlugin, list->x(), list->y()+(i*24), list->w());
	list->end();

	end();

	char name[256];
	pPlugin->getProduct(name);
	if (strcmp(name, " ")==0)
		pPlugin->getName(name);
	label(name);

	size_range(400, (24*1)+12);
	resizable(list);

	gu_setFavicon(this);
	show();

}


gdPluginWindow::~gdPluginWindow() {}


#endif // #ifdef WITH_VST
