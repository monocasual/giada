/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "src/gui/dialogs/pluginList.h"
#include "src/core/mixer.h"
#include "src/glue/layout.h"
#include "src/gui/elems/basics/liquidScroll.h"
#include "src/gui/elems/basics/textButton.h"
#include "src/gui/elems/mainWindow/keyboard/channel.h"
#include "src/gui/elems/plugin/pluginElement.h"
#include "src/gui/ui.h"
#include "src/utils/gui.h"

extern giada::v::Ui* g_ui;

namespace giada::v
{
gdPluginList::gdPluginList(ID channelId, geompp::Rect<int> bounds)
: gdWindow(u::gui::getCenterWinBounds(bounds), "", WID_FX_LIST)
, m_channelId(channelId)
{
	end();

	list = new geLiquidScroll(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN,
	    w() - (G_GUI_OUTER_MARGIN * 2), h() - (G_GUI_OUTER_MARGIN * 2),
	    Direction::VERTICAL);
	list->end();
	add(list);
	resizable(list);

	set_non_modal();
	rebuild();
	show();
}

/* -------------------------------------------------------------------------- */

gdPluginList::~gdPluginList()
{
	g_ui->model.pluginListBounds = getBounds();
}

/* -------------------------------------------------------------------------- */

void gdPluginList::rebuild()
{
	m_plugins = c::plugin::getPlugins(m_channelId);

	if (m_plugins.channelId == m::Mixer::MASTER_OUT_CHANNEL_ID)
		label(g_ui->getI18Text(LangMap::PLUGINLIST_TITLE_MASTEROUT));
	else if (m_plugins.channelId == m::Mixer::MASTER_IN_CHANNEL_ID)
		label(g_ui->getI18Text(LangMap::PLUGINLIST_TITLE_MASTERIN));
	else
		label(g_ui->getI18Text(LangMap::PLUGINLIST_TITLE_CHANNEL));

	/* Clear the previous list. */

	list->clear();
	list->scroll_to(0, 0);

	for (m::Plugin* plugin : m_plugins.plugins)
		list->addWidget(new gePluginElement(0, 0, w(), G_GUI_UNIT, c::plugin::getPlugin(*plugin, m_plugins.channelId)));

	addPlugin = new geTextButton(0, 0, 0, G_GUI_UNIT, g_ui->getI18Text(LangMap::PLUGINLIST_ADDPLUGIN));

	addPlugin->onClick = [this]()
	{
		c::layout::openPluginChooser(m_plugins.channelId);
	};

	list->addWidget(addPlugin);
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