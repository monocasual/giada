/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#include "gui/dialogs/pluginList.h"
#include "core/conf.h"
#include "core/const.h"
#include "glue/layout.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/liquidScroll.h"
#include "gui/elems/basics/statusButton.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/elems/mainWindow/mainIO.h"
#include "gui/elems/plugin/pluginElement.h"
#include "utils/gui.h"
#include "utils/string.h"
#include <cassert>
#include <string>

namespace giada::v
{
gdPluginList::gdPluginList(ID channelId, m::Conf::Data& c)
: gdWindow(c.pluginListX, c.pluginListY, 468, 204)
, m_conf(c)
, m_channelId(channelId)
{
	end();

	list = new geLiquidScroll(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN,
	    w() - (G_GUI_OUTER_MARGIN * 2), h() - (G_GUI_OUTER_MARGIN * 2),
	    Direction::VERTICAL);
	list->end();
	add(list);
	resizable(list);

	u::gui::setFavicon(this);
	set_non_modal();
	rebuild();
	show();
}

/* -------------------------------------------------------------------------- */

gdPluginList::~gdPluginList()
{
	m_conf.pluginListX = x();
	m_conf.pluginListY = y();
}

/* -------------------------------------------------------------------------- */

void gdPluginList::cb_addPlugin(Fl_Widget* /*v*/, void* p) { ((gdPluginList*)p)->cb_addPlugin(); }

/* -------------------------------------------------------------------------- */

void gdPluginList::rebuild()
{
	m_plugins = c::plugin::getPlugins(m_channelId);

	if (m_plugins.channelId == m::Mixer::MASTER_OUT_CHANNEL_ID)
		label("Master Out Plug-ins");
	else if (m_plugins.channelId == m::Mixer::MASTER_IN_CHANNEL_ID)
		label("Master In Plug-ins");
	else
	{
		std::string l = "Channel " + u::string::iToString(m_plugins.channelId) + " Plug-ins";
		copy_label(l.c_str());
	}

	/* Clear the previous list. */

	list->clear();
	list->scroll_to(0, 0);

	for (m::Plugin* plugin : m_plugins.plugins)
		list->addWidget(new gePluginElement(0, 0, c::plugin::getPlugin(*plugin, m_plugins.channelId)));

	addPlugin = list->addWidget(new geButton(0, 0, 0, G_GUI_UNIT, "-- add new plugin --"));

	addPlugin->callback(cb_addPlugin, (void*)this);
}

/* -------------------------------------------------------------------------- */

void gdPluginList::cb_addPlugin()
{
	c::layout::openPluginChooser(m_plugins.channelId);
}

/* -------------------------------------------------------------------------- */

const gePluginElement& gdPluginList::getNextElement(const gePluginElement& currEl) const
{
	int curr = list->find(currEl);
	int next = curr + 1;
	if (next > list->countChildren() - 2)
		next = list->countChildren() - 2;
	return *static_cast<gePluginElement*>(list->child(next));
}

const gePluginElement& gdPluginList::getPrevElement(const gePluginElement& currEl) const
{
	int curr = list->find(currEl);
	int prev = curr - 1;
	if (prev < 0)
		prev = 0;
	return *static_cast<gePluginElement*>(list->child(prev));
}
} // namespace giada::v

#endif // #ifdef WITH_VST
