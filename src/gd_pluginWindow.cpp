/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_pluginWindow
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2012 Giovanni A. Zuliani | Monocasual
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

#include "gd_pluginWindow.h"


extern PluginHost G_PluginHost;


gdPluginWindow::gdPluginWindow(Plugin *pPlugin)
 : gWindow(450, 300), pPlugin(pPlugin) {

	int numParams = pPlugin->getNumParams();
	int i = 0;

	/* before printing the paramName (which of course is not standard), we
	 * must check its length, so that we can shift the block on the right
	 * in order to make room for it.
	 *  [paramName] [slider] [paramDisplay] */

	int dx = 0;
	for (int i=0; i<numParams; i++) {
		char name[256];
		pPlugin->getParamName(i, name);
		if (fl_width(name) > dx)
			dx = fl_width(name)+25; // fl_width is not 100% precise, at least on Linux
	}

	Fl_Scroll *list = new Fl_Scroll(8, 8, w()-16, h()-16);
	list->type(Fl_Scroll::VERTICAL);
	list->scrollbar.color(COLOR_BG_0);
	list->scrollbar.selection_color(COLOR_BG_1);
	list->scrollbar.labelcolor(COLOR_BD_1);
	list->scrollbar.slider(G_BOX);

	list->begin();

	while (i<numParams) {

		/* y = i*20 [widget heigth] + i*4 [padding] + 8 [border top] */

		gBox    *label = new gBox   (list->x(), i*24+8, dx,  20);
		gSlider *sl    = new gSlider(dx+12,     i*24+8, 278, 20);
		gBox    *value = new gBox   (278+dx+16, i*24+8, 80,  20);

		/* some vsts don't follow kVstMaxParamStrLen for the params' length.
		 * The text value is made of paramDisplay and paramLabel. Example:
		 * -3 dB = -3 (display) + dB (label). */

		char name[256];
		pPlugin->getParamName(i, name);
		char disp[256];
		char labl[256];
		pPlugin->getParamDisplay(i, disp);
		pPlugin->getParamLabel(i, labl);
		sprintf(disp, "%s %s", disp, labl);

		label->copy_label(name);
		label->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

		sl->value(pPlugin->getParam(i));
		sl->callback(cb_setValue, (void *)this);
		sl->id = i;

		value->copy_label(disp);
		value->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

		sliders.add(sl);
		values.add(value);

		i++;
	}

	list->end();

	end();

	/* if parameters' height < N, reduce the window. Otherwise the window
	 * remains fixed with a new scrollbar on the right. */

	if (i*24+8 < h()) {
		list->size(dx+278+80+24, i*24+8+4);
		this->size(dx+278+80+24, i*24+8+4);
	}
	else {
		list->size(dx+278+80+24, h()-16);
		this->size(dx+278+80+40, h());
	}

	char name[256];
	pPlugin->getProduct(name);
	if (strcmp(name, " ")==0)
		pPlugin->getName(name);
	label(name);
	gu_setFavicon(this);
	//callback(__cb_window_closer, (void*)this);
	set_non_modal();
	show();

}



gdPluginWindow::~gdPluginWindow() {}



void gdPluginWindow::cb_setValue(Fl_Widget *v, void *p)  { ((gdPluginWindow*)p)->__cb_setValue(v); }



void gdPluginWindow::__cb_setValue(Fl_Widget *v) {

	gSlider *s = (gSlider *) v;
	pPlugin->setParam(s->id, s->value());

	char disp[256];
	char labl[256];
	pPlugin->getParamDisplay(s->id, disp);
	pPlugin->getParamLabel(s->id, labl);
	sprintf(disp, "%s %s", disp, labl);

	(values.at(s->id))->copy_label(disp);
	(values.at(s->id))->redraw();

}

#endif // #ifdef WITH_VST
