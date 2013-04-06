/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_pluginList
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013 Giovanni A. Zuliani | Monocasual
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

#include "gd_pluginList.h"
#include "utils.h"
#include "gd_pluginWindow.h"
#include "gd_pluginWindowGUI.h"
#include "conf.h"
#include "gui_utils.h"
#include "gd_browser.h"
#include "graphics.h"
#include "pluginHost.h"
#include "ge_mixed.h"
#include "mixer.h"
#include "channel.h"


extern Conf       G_Conf;
extern PluginHost G_PluginHost;


gdPluginList::gdPluginList(int stackType, struct channel *ch)
 : gWindow(468, 204), ch(ch), stackType(stackType)
{

	if (G_Conf.pluginListX)
		resize(G_Conf.pluginListX, G_Conf.pluginListY, w(), h());

	list = new Fl_Scroll(8, 8, 476, 188);
	list->type(Fl_Scroll::VERTICAL);
	list->scrollbar.color(COLOR_BG_0);
	list->scrollbar.selection_color(COLOR_BG_1);
	list->scrollbar.labelcolor(COLOR_BD_1);
	list->scrollbar.slider(G_BOX);

	list->begin();
		refreshList();
	list->end();

	end();
	set_non_modal();

	if (stackType == PluginHost::MASTER_OUT)
		label("Master Out Plugins");
	else
	if (stackType == PluginHost::MASTER_IN)
		label("Master In Plugins");
	else {
		char tmp[32];
		sprintf(tmp, "Channel %d Plugins", ch->index+1);
		copy_label(tmp);
	}

	gu_setFavicon(this);
	show();
}


/* ------------------------------------------------------------------ */


gdPluginList::~gdPluginList() {
	G_Conf.pluginListX = x();
	G_Conf.pluginListY = y();
}


/* ------------------------------------------------------------------ */


void gdPluginList::cb_addPlugin       (Fl_Widget *v, void *p)    { ((gdPluginList*)p)->__cb_addPlugin(); }
void gdPluginList::cb_removePlugin    (Fl_Widget *v, void *p)    { ((gdPluginList*)p)->__cb_removePlugin(v); }
void gdPluginList::cb_openPluginWindow(Fl_Widget *v, void *p)    { ((gdPluginList*)p)->__cb_openPluginWindow(v); }
void gdPluginList::cb_setBypass       (Fl_Widget *v, void *p)    { ((gdPluginList*)p)->__cb_setBypass(v); }
void gdPluginList::cb_shiftUp         (Fl_Widget *v, void *p)    { ((gdPluginList*)p)->__cb_shiftUp(v); }
void gdPluginList::cb_shiftDown       (Fl_Widget *v, void *p)    { ((gdPluginList*)p)->__cb_shiftDown(v); }
void gdPluginList::cb_setProgram      (Fl_Widget *v, void *p)    { ((gdPluginList*)p)->__cb_setProgram(v); }


/* ------------------------------------------------------------------ */


void gdPluginList::__cb_shiftUp(Fl_Widget *v) {

	/*nothing to do if there's only one plugin */

	if (G_PluginHost.countPlugins(stackType, ch) == 1)
		return;

	int id  = ((gButton*)v)->id;
	int pId = G_PluginHost.getPluginIndex(id, stackType, ch);

	/* if the plugin is the first one of the stack, do nothing */

	if (pId == 0)
		return;

	G_PluginHost.swapPlugin(pId, pId-1, stackType, ch);
	refreshList();
	redraw();
}


/* ------------------------------------------------------------------ */


void gdPluginList::__cb_shiftDown(Fl_Widget *v) {

	/*nothing to do if there's only one plugin */

	if (G_PluginHost.countPlugins(stackType, ch) == 1)
		return;

	int id = ((gButton*)v)->id;
	unsigned index = G_PluginHost.getPluginIndex(id, stackType, ch);

	/* if the plugin is the first one of the stack, do nothing */

	if (index == G_PluginHost.masterOut.size-1)
		return;

	G_PluginHost.swapPlugin(index, index+1, stackType, ch);
	refreshList();
	redraw();
}


/* ------------------------------------------------------------------ */


void gdPluginList::cb_refreshList(Fl_Widget *v, void *p) {

	/* note: this callback is fired by gdBrowser. Close its window first,
	 * by calling the parent (pluginList) and telling it to delete its
	 * subwindow (i.e. gdBrowser). */

	gWindow *child = (gWindow*) v;
	if (child->getParent() != NULL)
		(child->getParent())->delSubWindow(child);

	/* finally refresh plugin list: void *p is a pointer to gdPluginList.
	 * This callback works even when you click 'x' to close the window...
	 * well, who cares */

	((gdPluginList*)p)->refreshList();
	((gdPluginList*)p)->redraw();
}


/* ------------------------------------------------------------------ */


void gdPluginList::__cb_addPlugin() {

	/* the usual callback that gWindow adds to each subwindow in this case
	 * is not enough, because when we close the browser the plugin list
	 * must be redrawn. We have a special callback, cb_refreshList, which
	 * we add to gdBrowser. It does exactly what we need. */

	gdBrowser *b = new gdBrowser("Browse Plugin", G_Conf.pluginPath, ch, BROWSER_LOAD_PLUGIN, stackType);
	addSubWindow(b);
	b->callback(cb_refreshList, (void*)this);	// 'this' refers to gdPluginList

}


/* ------------------------------------------------------------------ */


void gdPluginList::__cb_removePlugin(Fl_Widget *v) {

	int i = ((gButton*)v)->id;

	/* any subwindow linked to the plugin must be destroyed */

	delSubWindow(i+1);

	G_PluginHost.freePlugin(i, stackType, ch);

	refreshList();
	redraw();
}


/* ------------------------------------------------------------------ */


void gdPluginList::__cb_openPluginWindow(Fl_Widget *v) {

	/* take the button id: we need it to obtain a pointer to the right
	 * plugin and create an unique window id */

	int id    = ((gButton*)v)->id;

	Plugin *pPlugin = G_PluginHost.getPluginById(id, stackType, ch);

	/* the new pluginWindow has id = id_plugin + 1, because id=0 is reserved
	 * for the window 'add plugin'. */

	/* TODO - at the moment you can open a window for each plugin in the stack.
	 * This is not consistent with the rest of the gui. You can avoid this by
	 * calling
	 *
	 * gu_openSubWindow(this, new gdPluginWindow(pPlugin), WID_FX);
	 *
	 * instead of the following code. */

	if (!hasWindow(id+1)) {
		gWindow *w;
		if (pPlugin->hasGui())
			w = new gdPluginWindowGUI(pPlugin);
		else
			w = new gdPluginWindow(pPlugin);
		w->setId(id+1);
		addSubWindow(w);
	}
}


/* ------------------------------------------------------------------ */


void gdPluginList::refreshList() {

	/* delete the previous list */

	list->clear();

	/* add new buttons, as many as the plugin in pluginHost::stack + 1,
	 * the 'add new' button. Warning: if ch == NULL we are working with
	 * master in/master out stacks. */

	int numPlugins = G_PluginHost.countPlugins(stackType, ch);
	int i = 0;
	while (i<numPlugins) {

		gButton *button    = new gButton(8,   i*20+8+i*4, 220, 20); // button contains an index
		gChoice *program   = new gChoice(button->x()+button->w()+4, i*20+8+i*4, 132, 20);
		gButton *bypass    = new gButton(program->x()+program->w()+4, i*20+8+i*4, 20, 20);
		gButton *shiftUp   = new gButton(bypass->x()+bypass->w()+4, i*20+8+i*4, 20, 20, "", fxShiftUpOff_xpm, fxShiftUpOn_xpm);
		gButton *shiftDown = new gButton(shiftUp->x()+shiftUp->w()+4, i*20+8+i*4, 20, 20, "", fxShiftDownOff_xpm, fxShiftDownOn_xpm);
		gButton *remove    = new gButton(shiftDown->x()+shiftDown->w()+4, i*20+8+i*4, 20, 20, "", fxRemoveOff_xpm, fxRemoveOn_xpm);

		Plugin *pPlugin = G_PluginHost.getPluginByIndex(i, stackType, ch);

		/* plugin is dead */

		if (pPlugin->status != 1) {
			char name[256];
			sprintf(name, "* %s *", gBasename(pPlugin->pathfile).c_str());
			button->copy_label(name);
		}

		/* plugin is healthy */

		else {
			char name[256];
			pPlugin->getProduct(name);
			if (strcmp(name, " ")==0)
				pPlugin->getName(name);

			button->copy_label(name);
			button->callback(cb_openPluginWindow, (void*)this);
			button->id = pPlugin->getId();

			program->callback(cb_setProgram, (void*)this);
			program->id = pPlugin->getId();

			/* loading vst programs */
			/* FIXME - max programs = 128 (unknown source) */

			for (int i=0; i<64; i++) {
				char out[kVstMaxProgNameLen];
				pPlugin->getProgramName(i, out);
				for (int j=0; j<kVstMaxProgNameLen; j++)  // escape FLTK special chars
					if (out[j] == '/' || out[j] == '\\' || out[j] == '&' || out[j] == '_')
						out[j] = '-';
				if (strlen(out) > 0)
					program->add(out);
			}
			if (program->size() == 0) {
				program->add("-- no programs --\0");
				program->deactivate();
			}
			program->value(0);

			bypass->callback(cb_setBypass, (void*)this);
			bypass->type(FL_TOGGLE_BUTTON);
			bypass->value(pPlugin->bypass ? 0 : 1);
			bypass->id = pPlugin->getId();
		}

		shiftUp->callback(cb_shiftUp, (void*)this);
		shiftDown->callback(cb_shiftDown, (void*)this);
		shiftUp->id = pPlugin->getId();
		shiftDown->id = pPlugin->getId();

		remove->callback(cb_removePlugin, (void*)this);
		remove->id = pPlugin->getId();

		list->add(button);
		list->add(program);
		list->add(remove);
		list->add(bypass);
		list->add(shiftUp);
		list->add(shiftDown);

		i++;
	}

	int addPlugY = numPlugins == 0 ? 90 : i*20+8+i*4;
	addPlugin = new gClick(8, addPlugY, 452, 20, "-- add new plugin --");
	addPlugin->callback(cb_addPlugin, (void*)this);
	list->add(addPlugin);

	/* if num(plugins) > 7 make room for the side scrollbar.
	 * Scrollbar.width = 20 + 4(margin) */

	if (i>7)
		size(492, h());
	else
		size(468, h());
}


/* ------------------------------------------------------------------ */


void gdPluginList::__cb_setBypass(Fl_Widget *w) {
	int id = ((gButton*)w)->id;
	Plugin *pPlugin = G_PluginHost.getPluginById(id, stackType, ch);
	pPlugin->bypass = !pPlugin->bypass;
}


/* ------------------------------------------------------------------ */


void gdPluginList::__cb_setProgram(Fl_Widget *w) {
	int id = ((gChoice*)w)->id;
	Plugin *pPlugin = G_PluginHost.getPluginById(id, stackType, ch);
	pPlugin->setProgram(((gChoice*)w)->value());
}


#endif // #ifdef WITH_VST
