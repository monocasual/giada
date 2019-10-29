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


#include <cassert>
#include <string>
#include "core/channels/channel.h"
#include "core/model/model.h"
#include "core/graphics.h"
#include "core/pluginHost.h"
#include "core/plugin.h"
#include "utils/gui.h"
#include "utils/log.h"
#include "glue/plugin.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/choice.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/pluginList.h"
#include "gui/dialogs/pluginWindowGUI.h"
#include "gui/dialogs/pluginWindow.h"
#include "pluginElement.h"


namespace giada {
namespace v
{
gePluginElement::gePluginElement(ID pluginId, ID channelId, int X, int Y, int W)
: Fl_Pack    (X, Y, W, G_GUI_UNIT), 
  m_channelId(channelId),
  m_pluginId (pluginId)
{
	type(Fl_Pack::HORIZONTAL);
	spacing(G_GUI_INNER_MARGIN);
	begin();
		button    = new geButton(0, 0, 196, G_GUI_UNIT);
		program   = new geChoice(0, 0, 132, G_GUI_UNIT);
		bypass    = new geButton(0, 0, G_GUI_UNIT, G_GUI_UNIT);
		shiftUp   = new geButton(0, 0, G_GUI_UNIT, G_GUI_UNIT, "", fxShiftUpOff_xpm, fxShiftUpOn_xpm);
		shiftDown = new geButton(0, 0, G_GUI_UNIT, G_GUI_UNIT, "", fxShiftDownOff_xpm, fxShiftDownOn_xpm);
		remove    = new geButton(0, 0, G_GUI_UNIT, G_GUI_UNIT, "", fxRemoveOff_xpm, fxRemoveOn_xpm);
	end();

	m::model::PluginsLock l(m::model::plugins);

	const m::Plugin& p = m::model::get(m::model::plugins, m_pluginId);

	button->copy_label(p.getName().c_str());
	button->callback(cb_openPluginWindow, (void*)this);

	program->callback(cb_setProgram, (void*)this);

	for (int i=0; i<p.getNumPrograms(); i++)
		program->add(u::gui::removeFltkChars(p.getProgramName(i)).c_str());

	if (program->size() == 0) {
		program->add("-- no programs --\0");
		program->deactivate();
	}
	else
		program->value(p.getCurrentProgram());

	bypass->callback(cb_setBypass, (void*)this);
	bypass->type(FL_TOGGLE_BUTTON);
	bypass->value(p.isBypassed() ? 0 : 1);

	shiftUp->callback(cb_shiftUp, (void*)this);
	shiftDown->callback(cb_shiftDown, (void*)this);
	remove->callback(cb_removePlugin, (void*)this);
}


/* -------------------------------------------------------------------------- */


ID gePluginElement::getPluginId() const
{
	return m_pluginId;
}


/* -------------------------------------------------------------------------- */


void gePluginElement::cb_removePlugin    (Fl_Widget* v, void* p) { ((gePluginElement*)p)->cb_removePlugin(); }
void gePluginElement::cb_openPluginWindow(Fl_Widget* v, void* p) { ((gePluginElement*)p)->cb_openPluginWindow(); }
void gePluginElement::cb_setBypass       (Fl_Widget* v, void* p) { ((gePluginElement*)p)->cb_setBypass(); }
void gePluginElement::cb_shiftUp         (Fl_Widget* v, void* p) { ((gePluginElement*)p)->cb_shiftUp(); }
void gePluginElement::cb_shiftDown       (Fl_Widget* v, void* p) { ((gePluginElement*)p)->cb_shiftDown(); }
void gePluginElement::cb_setProgram      (Fl_Widget* v, void* p) { ((gePluginElement*)p)->cb_setProgram(); }


/* -------------------------------------------------------------------------- */


void gePluginElement::cb_shiftUp()
{
	const gdPluginList* parent = static_cast<const gdPluginList*>(window());

	c::plugin::swapPlugins(m_pluginId, parent->getPrevElement(*this).getPluginId(), m_channelId);
}


/* -------------------------------------------------------------------------- */


void gePluginElement::cb_shiftDown()
{
	const gdPluginList* parent = static_cast<const gdPluginList*>(window());

	c::plugin::swapPlugins(m_pluginId, parent->getNextElement(*this).getPluginId(), m_channelId);
}


/* -------------------------------------------------------------------------- */


void gePluginElement::cb_removePlugin()
{
	/* Any subwindow linked to the plugin must be destroyed first. The 
	pluginWindow has id = id_plugin + 1, because id=0 is reserved for the parent 
	window 'add plugin'.*/
	
	static_cast<gdWindow*>(window())->delSubWindow(m_pluginId + 1);
	c::plugin::freePlugin(m_pluginId, m_channelId);
}


/* -------------------------------------------------------------------------- */


void gePluginElement::cb_openPluginWindow()
{
	m::model::PluginsLock l(m::model::plugins);

	const m::Plugin& p = m::model::get(m::model::plugins, m_pluginId);

	/* The new pluginWindow has id = id_plugin + 1, because id=0 is reserved for 
	the parent window 'add plugin'. */

	int pwid = m_pluginId + 1;

	gdWindow* parent = static_cast<gdWindow*>(window());
	gdWindow* child  = parent->getChild(pwid);

	if (child != nullptr) {
		child->show();  // Raise it to top
	}
	else {
		if (p.hasEditor())
			child = new gdPluginWindowGUI(m_pluginId);
		else 
			child = new gdPluginWindow(m_pluginId);
		child->setId(pwid);
		parent->addSubWindow(child);
	}
}


/* -------------------------------------------------------------------------- */


void gePluginElement::cb_setBypass()
{
	c::plugin::toggleBypass(m_pluginId);
}


/* -------------------------------------------------------------------------- */


void gePluginElement::cb_setProgram()
{
	c::plugin::setProgram(m_pluginId, program->value());
}
}} // giada::v::


#endif // #ifdef WITH_VST
