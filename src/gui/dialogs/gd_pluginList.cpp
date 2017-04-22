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


#include "../../utils/gui.h"
#include "../../utils/fs.h"
#include "../../core/conf.h"
#include "../../core/const.h"
#include "../../core/graphics.h"
#include "../../core/pluginHost.h"
#include "../../core/plugin.h"
#include "../../core/mixer.h"
#include "../../core/channel.h"
#include "../../glue/plugin.h"
#include "../../utils/log.h"
#include "../../utils/string.h"
#include "../elems/basics/boxtypes.h"
#include "../elems/basics/idButton.h"
#include "../elems/basics/statusButton.h"
#include "../elems/basics/choice.h"
#include "../elems/mainWindow/mainIO.h"
#include "../elems/mainWindow/keyboard/channel.h"
#include "gd_pluginList.h"
#include "gd_pluginChooser.h"
#include "gd_pluginWindow.h"
#include "gd_pluginWindowGUI.h"
#include "gd_browser.h"
#include "gd_mainWindow.h"


extern gdMainWindow *G_MainWin;


using std::string;
using namespace giada;


gdPluginList::gdPluginList(int stackType, Channel *ch)
  : gdWindow(468, 204), ch(ch), stackType(stackType)
{
	if (conf::pluginListX)
		resize(conf::pluginListX, conf::pluginListY, w(), h());

	list = new Fl_Scroll(8, 8, 476, 188);
	list->type(Fl_Scroll::VERTICAL);
	list->scrollbar.color(COLOR_BG_0);
	list->scrollbar.selection_color(COLOR_BG_1);
	list->scrollbar.labelcolor(COLOR_BD_1);
	list->scrollbar.slider(G_CUSTOM_BORDER_BOX);

	list->begin();
		refreshList();
	list->end();

	end();
	set_non_modal();

  /* TODO - awful stuff... we should subclass into gdPluginListChannel and
  gdPluginListMaster */

	if (stackType == pluginHost::MASTER_OUT)
		label("Master Out Plugins");
	else
	if (stackType == pluginHost::MASTER_IN)
		label("Master In Plugins");
	else {
    string l = "Channel " + gu_itoa(ch->index+1) + " Plugins";
    copy_label(l.c_str());
	}

	gu_setFavicon(this);
	show();
}


/* -------------------------------------------------------------------------- */


gdPluginList::~gdPluginList()
{
	conf::pluginListX = x();
	conf::pluginListY = y();
}


/* -------------------------------------------------------------------------- */


void gdPluginList::cb_addPlugin(Fl_Widget *v, void *p)   { ((gdPluginList*)p)->__cb_addPlugin(); }


/* -------------------------------------------------------------------------- */


void gdPluginList::cb_refreshList(Fl_Widget *v, void *p)
{
	/* note: this callback is fired by gdBrowser. Close its window first,
	 * by calling the parent (pluginList) and telling it to delete its
	 * subwindow (i.e. gdBrowser). */

	gdWindow *child = (gdWindow*) v;
	if (child->getParent() != nullptr)
		(child->getParent())->delSubWindow(child);

	/* finally refresh plugin list: void *p is a pointer to gdPluginList.
	 * This callback works even when you click 'x' to close the window...
	 * well, who cares */

	((gdPluginList*)p)->refreshList();
	((gdPluginList*)p)->redraw();
}


/* -------------------------------------------------------------------------- */


void gdPluginList::__cb_addPlugin()
{
	/* the usual callback that gdWindow adds to each subwindow in this case
	 * is not enough, because when we close the browser the plugin list
	 * must be redrawn. We have a special callback, cb_refreshList, which
	 * we add to gdPluginChooser. It does exactly what we need. */

  gdPluginChooser *pc = new gdPluginChooser(conf::pluginChooserX,
      conf::pluginChooserY, conf::pluginChooserW, conf::pluginChooserH,
      stackType, ch);
  addSubWindow(pc);
  pc->callback(cb_refreshList, (void*)this);	// 'this' refers to gdPluginList
}


/* -------------------------------------------------------------------------- */


void gdPluginList::refreshList()
{
	/* delete the previous list */

	list->clear();
	list->scroll_to(0, 0);

	/* add new buttons, as many as the plugin in pluginHost::stack + 1,
	 * the 'add new' button. Warning: if ch == nullptr we are working with
	 * master in/master out stacks. */

	int numPlugins = pluginHost::countPlugins(stackType, ch);
	int i = 0;

	while (i<numPlugins) {
		Plugin   *pPlugin = pluginHost::getPluginByIndex(i, stackType, ch);
		gdPlugin *gdp     = new gdPlugin(this, pPlugin, list->x(), list->y()-list->yposition()+(i*24), 800);
		list->add(gdp);
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

	if (stackType == pluginHost::MASTER_OUT) {
    G_MainWin->mainIO->setMasterFxOutFull(
			pluginHost::countPlugins(stackType, ch) > 0);
  }
	else
	if (stackType == pluginHost::MASTER_IN) {
    G_MainWin->mainIO->setMasterFxInFull(
			pluginHost::countPlugins(stackType, ch) > 0);
  }
	else {
    ch->guiChannel->fx->status = pluginHost::countPlugins(stackType, ch) > 0;
    ch->guiChannel->fx->redraw();
  }
}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


gdPlugin::gdPlugin(gdPluginList *gdp, Plugin *p, int X, int Y, int W)
	: Fl_Group(X, Y, W, 20), pParent(gdp), pPlugin (p)
{
	begin();
	button    = new geIdButton(8, y(), 220, 20);
	program   = new geChoice(button->x()+button->w()+4, y(), 132, 20);
	bypass    = new geIdButton(program->x()+program->w()+4, y(), 20, 20);
	shiftUp   = new geIdButton(bypass->x()+bypass->w()+4, y(), 20, 20, "", fxShiftUpOff_xpm, fxShiftUpOn_xpm);
	shiftDown = new geIdButton(shiftUp->x()+shiftUp->w()+4, y(), 20, 20, "", fxShiftDownOff_xpm, fxShiftDownOn_xpm);
	remove    = new geIdButton(shiftDown->x()+shiftDown->w()+4, y(), 20, 20, "", fxRemoveOff_xpm, fxRemoveOn_xpm);
	end();

	button->copy_label(pPlugin->getName().c_str());
	button->callback(cb_openPluginWindow, (void*)this);

	program->callback(cb_setProgram, (void*)this);

  for (int i=0; i<pPlugin->getNumPrograms(); i++)
    program->add(gu_removeFltkChars(pPlugin->getProgramName(i)).c_str());

	if (program->size() == 0) {
		program->add("-- no programs --\0");
		program->deactivate();
	}
  else
    program->value(pPlugin->getCurrentProgram());

	bypass->callback(cb_setBypass, (void*)this);
	bypass->type(FL_TOGGLE_BUTTON);
	bypass->value(pPlugin->isBypassed() ? 0 : 1);

	shiftUp->callback(cb_shiftUp, (void*)this);
	shiftDown->callback(cb_shiftDown, (void*)this);
	remove->callback(cb_removePlugin, (void*)this);
}


/* -------------------------------------------------------------------------- */


void gdPlugin::cb_removePlugin    (Fl_Widget *v, void *p)    { ((gdPlugin*)p)->__cb_removePlugin(); }
void gdPlugin::cb_openPluginWindow(Fl_Widget *v, void *p)    { ((gdPlugin*)p)->__cb_openPluginWindow(); }
void gdPlugin::cb_setBypass       (Fl_Widget *v, void *p)    { ((gdPlugin*)p)->__cb_setBypass(); }
void gdPlugin::cb_shiftUp         (Fl_Widget *v, void *p)    { ((gdPlugin*)p)->__cb_shiftUp(); }
void gdPlugin::cb_shiftDown       (Fl_Widget *v, void *p)    { ((gdPlugin*)p)->__cb_shiftDown(); }
void gdPlugin::cb_setProgram      (Fl_Widget *v, void *p)    { ((gdPlugin*)p)->__cb_setProgram(); }


/* -------------------------------------------------------------------------- */


void gdPlugin::__cb_shiftUp()
{
	/*nothing to do if there's only one plugin */

	if (pluginHost::countPlugins(pParent->stackType, pParent->ch) == 1)
		return;

	int pluginIndex = pluginHost::getPluginIndex(pPlugin->getId(),
    pParent->stackType, pParent->ch);

	if (pluginIndex == 0)  // first of the stack, do nothing
		return;

  glue_swapPlugins(pParent->ch, pluginIndex, pluginIndex-1, pParent->stackType);
	pParent->refreshList();
}


/* -------------------------------------------------------------------------- */


void gdPlugin::__cb_shiftDown()
{
	/*nothing to do if there's only one plugin */

	if (pluginHost::countPlugins(pParent->stackType, pParent->ch) == 1)
		return;

	unsigned pluginIndex = pluginHost::getPluginIndex(pPlugin->getId(), pParent->stackType, pParent->ch);
	unsigned stackSize   = (pluginHost::getStack(pParent->stackType, pParent->ch))->size();

	if (pluginIndex == stackSize-1)  // last one in the stack, do nothing
		return;

  glue_swapPlugins(pParent->ch, pluginIndex, pluginIndex+1, pParent->stackType);
	pParent->refreshList();
}


/* -------------------------------------------------------------------------- */


void gdPlugin::__cb_removePlugin()
{
	/* any subwindow linked to the plugin must be destroyed first */

	pParent->delSubWindow(pPlugin->getId());
  glue_freePlugin(pParent->ch, pPlugin->getId(), pParent->stackType);
  pParent->refreshList();
}


/* -------------------------------------------------------------------------- */


void gdPlugin::__cb_openPluginWindow()
{
  /* the new pluginWindow has id = id_plugin + 1, because id=0 is reserved
  * for the parent window 'add plugin'. */

  gdWindow *w;
  if (pPlugin->hasEditor()) {
    if (pPlugin->isEditorOpen()) {
      gu_log("[gdPlugin::__cb_openPluginWindow] plugin has editor but it's already visible\n");
      return;
    }

    int pwid = pPlugin->getId()+1;

    gu_log("[gdPlugin::__cb_openPluginWindow] plugin has editor, open window id=%d\n", pwid);

    if (pParent->hasWindow(pwid))
      pParent->delSubWindow(pwid);
    w = new gdPluginWindowGUI(pPlugin);
    w->setId(pwid);
		pParent->addSubWindow(w);
  }
  else {
    w = new gdPluginWindow(pPlugin);
  }
}


/* -------------------------------------------------------------------------- */


void gdPlugin::__cb_setBypass()
{
	pPlugin->toggleBypass();
}


/* -------------------------------------------------------------------------- */


void gdPlugin::__cb_setProgram()
{
	pPlugin->setCurrentProgram(program->value());
}


#endif // #ifdef WITH_VST
