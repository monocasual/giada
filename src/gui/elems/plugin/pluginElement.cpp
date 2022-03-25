/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "pluginElement.h"
#include "core/graphics.h"
#include "core/plugins/plugin.h"
#include "core/plugins/pluginHost.h"
#include "glue/plugin.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/pluginList.h"
#include "gui/dialogs/pluginWindow.h"
#include "gui/dialogs/pluginWindowGUI.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/choice.h"
#include "utils/gui.h"
#include "utils/log.h"
#include <cassert>
#include <string>

namespace giada::v
{
gePluginElement::gePluginElement(int x, int y, c::plugin::Plugin data)
: gePack(x, y, Direction::HORIZONTAL)
, button(0, 0, 196, G_GUI_UNIT)
, program(0, 0, 132, G_GUI_UNIT)
, bypass(0, 0, G_GUI_UNIT, G_GUI_UNIT)
, shiftUp(0, 0, G_GUI_UNIT, G_GUI_UNIT, "", fxShiftUpOff_xpm, fxShiftUpOn_xpm)
, shiftDown(0, 0, G_GUI_UNIT, G_GUI_UNIT, "", fxShiftDownOff_xpm, fxShiftDownOn_xpm)
, remove(0, 0, G_GUI_UNIT, G_GUI_UNIT, "", fxRemoveOff_xpm, fxRemoveOn_xpm)
, m_plugin(data)
{
	add(&button);
	add(&program);
	add(&bypass);
	add(&shiftUp);
	add(&shiftDown);
	add(&remove);

	resizable(button);

	remove.callback(cb_removePlugin, (void*)this);

	if (!m_plugin.valid)
	{
		button.copy_label(m_plugin.uniqueId.c_str());
		button.deactivate();
		bypass.deactivate();
		shiftUp.deactivate();
		shiftDown.deactivate();
		return;
	}

	button.copy_label(m_plugin.name.c_str());
	button.callback(cb_openPluginWindow, (void*)this);

	program.onChange = [pluginId = m_plugin.id](ID id) {
		c::plugin::setProgram(pluginId, id);
	};

	for (const auto& p : m_plugin.programs)
		program.addItem(u::gui::removeFltkChars(p.name));

	if (program.countItems() == 0)
	{
		program.addItem("-- no programs --\0");
		program.deactivate();
	}
	else
		program.showItem(m_plugin.currentProgram);

	bypass.callback(cb_setBypass, (void*)this);
	bypass.type(FL_TOGGLE_BUTTON);
	bypass.value(m_plugin.isBypassed ? 0 : 1);

	shiftUp.callback(cb_shiftUp, (void*)this);
	shiftDown.callback(cb_shiftDown, (void*)this);
}

/* -------------------------------------------------------------------------- */

ID gePluginElement::getPluginId() const
{
	return m_plugin.id;
}

const m::Plugin& gePluginElement::getPluginRef() const
{
	return m_plugin.getPluginRef();
}

/* -------------------------------------------------------------------------- */

void gePluginElement::cb_removePlugin(Fl_Widget* /*w*/, void* p) { ((gePluginElement*)p)->cb_removePlugin(); }
void gePluginElement::cb_openPluginWindow(Fl_Widget* /*w*/, void* p) { ((gePluginElement*)p)->cb_openPluginWindow(); }
void gePluginElement::cb_setBypass(Fl_Widget* /*w*/, void* p) { ((gePluginElement*)p)->cb_setBypass(); }
void gePluginElement::cb_shiftUp(Fl_Widget* /*w*/, void* p) { ((gePluginElement*)p)->cb_shiftUp(); }
void gePluginElement::cb_shiftDown(Fl_Widget* /*w*/, void* p) { ((gePluginElement*)p)->cb_shiftDown(); }

/* -------------------------------------------------------------------------- */

void gePluginElement::cb_shiftUp()
{
	const gdPluginList* parent = static_cast<const gdPluginList*>(window());

	c::plugin::swapPlugins(m_plugin.getPluginRef(), parent->getPrevElement(*this).getPluginRef(), m_plugin.channelId);
}

/* -------------------------------------------------------------------------- */

void gePluginElement::cb_shiftDown()
{
	const gdPluginList* parent = static_cast<const gdPluginList*>(window());

	c::plugin::swapPlugins(m_plugin.getPluginRef(), parent->getNextElement(*this).getPluginRef(), m_plugin.channelId);
}

/* -------------------------------------------------------------------------- */

void gePluginElement::cb_removePlugin()
{
	/* Any subwindow linked to the plugin must be destroyed first. The 
	pluginWindow has id = id_plugin + 1, because id=0 is reserved for the parent 
	window 'add plugin'.*/

	static_cast<gdWindow*>(window())->delSubWindow(m_plugin.id + 1);
	c::plugin::freePlugin(m_plugin.getPluginRef(), m_plugin.channelId);
}

/* -------------------------------------------------------------------------- */

void gePluginElement::cb_openPluginWindow()
{
	/* The new pluginWindow has id = id_plugin + 1, because id=0 is reserved for 
	the parent window 'add plugin'. */

	const int pwid = m_plugin.id + 1;

	gdWindow* parent = static_cast<gdWindow*>(window());
	gdWindow* child  = parent->getChild(pwid);

	/* If Plug-in window is already opened, just raise it on top and quit. */

	if (child != nullptr)
	{
		child->show();
		return;
	}

	if (m_plugin.hasEditor)
		child = new gdPluginWindowGUI(m_plugin);
	else
		child = new gdPluginWindow(m_plugin);
	child->setId(pwid);
	parent->addSubWindow(child);
}

/* -------------------------------------------------------------------------- */

void gePluginElement::cb_setBypass()
{
	c::plugin::toggleBypass(m_plugin.id);
}
} // namespace giada::v

#endif // #ifdef WITH_VST
