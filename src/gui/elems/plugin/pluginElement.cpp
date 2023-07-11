/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "gui/elems/plugin/pluginElement.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/pluginList.h"
#include "gui/dialogs/pluginWindow.h"
#include "gui/dialogs/pluginWindowGUI.h"
#include "gui/elems/basics/choice.h"
#include "gui/elems/basics/imageButton.h"
#include "gui/elems/basics/pack.h"
#include "gui/elems/basics/textButton.h"
#include "gui/graphics.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include "utils/log.h"
#include <cassert>
#include <string>

extern giada::v::Ui g_ui;

namespace giada::v
{
gePluginElement::gePluginElement(int x, int y, int w, int h, c::plugin::Plugin data)
: geFlex(x, y, w, h, Direction::HORIZONTAL, G_GUI_INNER_MARGIN)
, m_plugin(data)
{
	button       = new geTextButton("");
	program      = new geChoice();
	bypass       = new geTextButton("");
	shiftUpBtn   = new geImageButton(graphics::upOff, graphics::upOn);
	shiftDownBtn = new geImageButton(graphics::downOff, graphics::downOn);
	remove       = new geImageButton(graphics::removeOff, graphics::removeOn);
	add(button);
	add(program);
	add(bypass, G_GUI_UNIT);
	add(shiftUpBtn, G_GUI_UNIT);
	add(shiftDownBtn, G_GUI_UNIT);
	add(remove, G_GUI_UNIT);
	end();

	remove->onClick = [this]() { removePlugin(); };

	if (!m_plugin.valid)
	{
		button->copy_label(m_plugin.uniqueId.c_str());
		button->deactivate();
		bypass->deactivate();
		shiftUpBtn->deactivate();
		shiftDownBtn->deactivate();
		return;
	}

	button->copy_label(m_plugin.name.c_str());
	button->onClick = [this]() { openPluginWindow(); };

	program->onChange = [pluginId = m_plugin.id](ID id) {
		c::plugin::setProgram(pluginId, id);
	};

	for (const auto& p : m_plugin.programs)
		program->addItem(u::gui::removeFltkChars(p.name));

	if (program->countItems() == 0)
	{
		program->addItem(g_ui.getI18Text(LangMap::PLUGINLIST_NOPROGRAMS));
		program->showItem(0);
		program->deactivate();
	}
	else
		program->showItem(m_plugin.currentProgram);

	bypass->setToggleable(true);
	bypass->setValue(!m_plugin.isBypassed);
	bypass->onClick = [this]() {
		c::plugin::toggleBypass(m_plugin.id);
	};

	shiftUpBtn->onClick   = [this]() { shiftUp(); };
	shiftDownBtn->onClick = [this]() { shiftDown(); };
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

void gePluginElement::shiftUp()
{
	const gdPluginList* parent = static_cast<const gdPluginList*>(window());

	c::plugin::swapPlugins(m_plugin.getPluginRef(), parent->getPrevElement(*this).getPluginRef(), m_plugin.channelId);
}

/* -------------------------------------------------------------------------- */

void gePluginElement::shiftDown()
{
	const gdPluginList* parent = static_cast<const gdPluginList*>(window());

	c::plugin::swapPlugins(m_plugin.getPluginRef(), parent->getNextElement(*this).getPluginRef(), m_plugin.channelId);
}

/* -------------------------------------------------------------------------- */

void gePluginElement::removePlugin()
{
	/* Any subwindow linked to the plugin must be destroyed first. The 
	pluginWindow has id = id_plugin + 1, because id=0 is reserved for the parent 
	window 'add plugin'.*/

	g_ui.closeSubWindow(m_plugin.id + 1);
	c::plugin::freePlugin(m_plugin.getPluginRef(), m_plugin.channelId);
}

/* -------------------------------------------------------------------------- */

void gePluginElement::openPluginWindow()
{
	/* The new pluginWindow has id = id_plugin + 1, because id=0 is reserved for 
	the parent window 'add plugin'. */

	const int pwid = m_plugin.id + 1;

	gdWindow* child = g_ui.getSubwindow(*g_ui.mainWindow.get(), pwid);

	/* If Plug-in window is already opened, just raise it on top and quit. */

	if (child != nullptr)
	{
		child->show();
		return;
	}

	if (m_plugin.hasEditor)
		child = new gdPluginWindowGUI(m_plugin, pwid);
	else
		child = new gdPluginWindow(m_plugin, pwid);

	g_ui.openSubWindow(child);
}
} // namespace giada::v