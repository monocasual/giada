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


#include <string>
#include <FL/Fl_Scroll.H>
#include "../../utils/gui.h"
#include "../../core/conf.h"
#include "../../core/const.h"
#include "../../core/pluginHost.h"
#include "../../core/channel.h"
#include "../../utils/string.h"
#include "../elems/basics/boxtypes.h"
#include "../elems/basics/button.h"
#include "../elems/basics/statusButton.h"
#include "../elems/mainWindow/mainIO.h"
#include "../elems/mainWindow/keyboard/channel.h"
#include "../elems/plugin/pluginElement.h"
#include "pluginChooser.h"
#include "gd_mainWindow.h"
#include "pluginList.h"


extern gdMainWindow* G_MainWin;


using std::string;
using namespace giada;


gdPluginList::gdPluginList(m::pluginHost::StackType t, m::Channel* ch)
	: gdWindow(468, 204), ch(ch), stackType(t)
{
	using namespace giada::m;

	if (conf::pluginListX)
		resize(conf::pluginListX, conf::pluginListY, w(), h());

	list = new Fl_Scroll(8, 8, 476, 188);
	list->type(Fl_Scroll::VERTICAL);
	list->scrollbar.color(G_COLOR_GREY_2);
	list->scrollbar.selection_color(G_COLOR_GREY_4);
	list->scrollbar.labelcolor(G_COLOR_LIGHT_1);
	list->scrollbar.slider(G_CUSTOM_BORDER_BOX);

	list->begin();
		refreshList();
	list->end();

	end();
	set_non_modal();

	/* TODO - awful stuff... we should subclass into gdPluginListChannel and
	gdPluginListMaster */

	if (stackType == pluginHost::StackType::MASTER_OUT)
		label("Master Out Plugins");
	else
	if (stackType == pluginHost::StackType::MASTER_IN)
		label("Master In Plugins");
	else {
		string l = "Channel " + gu_iToString(ch->index+1) + " Plugins";
		copy_label(l.c_str());
	}

	gu_setFavicon(this);
	show();
}


/* -------------------------------------------------------------------------- */


gdPluginList::~gdPluginList()
{
	m::conf::pluginListX = x();
	m::conf::pluginListY = y();
}


/* -------------------------------------------------------------------------- */


void gdPluginList::cb_addPlugin(Fl_Widget* v, void* p) { ((gdPluginList*)p)->cb_addPlugin(); }


/* -------------------------------------------------------------------------- */


void gdPluginList::cb_refreshList(Fl_Widget* v, void* p)
{
	/* Note: this callback is fired by gdBrowser. Close its window first, by 
	calling the parent (pluginList) and telling it to delete its subwindow 
	(i.e. gdBrowser). */

	gdWindow* child = static_cast<gdWindow*>(v);
	if (child->getParent() != nullptr)
		(child->getParent())->delSubWindow(child);

	/* Finally refresh plugin list: void *p is a pointer to gdPluginList. This 
	callback works even when you click 'x' to close the window... well, it does
	not matter. */

	((gdPluginList*)p)->refreshList();
	((gdPluginList*)p)->redraw();
}


/* -------------------------------------------------------------------------- */


void gdPluginList::cb_addPlugin()
{
	using namespace giada::m;

	/* The usual callback that gdWindow adds to each subwindow in this case is not 
	enough, because when we close the browser the plugin list must be redrawn. We 
	have a special callback, cb_refreshList, which we add to gdPluginChooser. 
	It does exactly what we need. */

	gdPluginChooser* pc = new gdPluginChooser(conf::pluginChooserX,
			conf::pluginChooserY, conf::pluginChooserW, conf::pluginChooserH,
			stackType, ch);
	addSubWindow(pc);
	pc->callback(cb_refreshList, (void*)this);	// 'this' refers to gdPluginList
}


/* -------------------------------------------------------------------------- */


void gdPluginList::refreshList()
{
	using namespace giada::m;

	/* delete the previous list */

	list->clear();
	list->scroll_to(0, 0);

	/* add new buttons, as many as the plugin in pluginHost::stack + 1,
	 * the 'add new' button. Warning: if ch == nullptr we are working with
	 * master in/master out stacks. */

	int numPlugins = pluginHost::countPlugins(stackType, ch);
	int i = 0;

	while (i<numPlugins) {
		Plugin*          plugin = pluginHost::getPluginByIndex(i, stackType, ch);
		gePluginElement* gdpe   = new gePluginElement(this, plugin, list->x(), 
			list->y()-list->yposition()+(i*24), 800);
		list->add(gdpe);
		i++;
	}

	int addPlugY = numPlugins == 0 ? 90 : list->y()-list->yposition()+(i*24);
	addPlugin = new geButton(8, addPlugY, 452, 20, "-- add new plugin --");
	addPlugin->callback(cb_addPlugin, (void*)this);
	list->add(addPlugin);

	/* if num(plugins) > 7 make room for the side scrollbar.
	 * Scrollbar.width = 20 + 4(margin) */

	if (i>7)
		size(492, h());
	else
		size(468, h());

	redraw();

	/* set 'full' flag to FX button */

	/* TODO - awful stuff... we should subclass into gdPluginListChannel and
	gdPluginListMaster */

	if (stackType == pluginHost::StackType::MASTER_OUT) {
		G_MainWin->mainIO->setMasterFxOutFull(pluginHost::countPlugins(stackType, ch) > 0);
	}
	else
	if (stackType == pluginHost::StackType::MASTER_IN) {
		G_MainWin->mainIO->setMasterFxInFull(pluginHost::countPlugins(stackType, ch) > 0);
	}
	else {
		ch->guiChannel->fx->status = pluginHost::countPlugins(stackType, ch) > 0;
		ch->guiChannel->fx->redraw();
	}
}


#endif // #ifdef WITH_VST
