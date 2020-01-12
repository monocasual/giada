/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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
#include "core/model/model.h"
#include "core/channels/channel.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/pluginHost.h"
#include "utils/string.h"
#include "utils/gui.h"
#include "gui/elems/basics/liquidScroll.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/statusButton.h"
#include "gui/elems/mainWindow/mainIO.h"
#include "gui/elems/mainWindow/keyboard/channel.h"
#include "gui/elems/plugin/pluginElement.h"
#include "pluginChooser.h"
#include "mainWindow.h"
#include "pluginList.h"


extern giada::v::gdMainWindow* G_MainWin;


namespace giada {
namespace v
{
gdPluginList::gdPluginList(ID chanID)
: gdWindow(m::conf::pluginListX, m::conf::pluginListY, 468, 204), 
  m_channelId(chanID)
{
	end();

	list = new geLiquidScroll(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN, 
		w() - (G_GUI_OUTER_MARGIN*2), h() - (G_GUI_OUTER_MARGIN*2));
	list->end();
	add(list);

	rebuild();

	if (m_channelId == m::mixer::MASTER_OUT_CHANNEL_ID)
		label("Master Out Plug-ins");
	else
	if (m_channelId == m::mixer::MASTER_IN_CHANNEL_ID)
		label("Master In Plug-ins");
	else {
		std::string l = "Channel " + u::string::iToString(m_channelId + 1) + " Plug-ins";
		copy_label(l.c_str());
	}

	u::gui::setFavicon(this);
	set_non_modal();
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


void gdPluginList::rebuild()
{
	/* Clear the previous list. */

	list->clear();
	list->scroll_to(0, 0);

	m::model::ChannelsLock l(m::model::channels);

	const m::Channel& ch = m::model::get(m::model::channels, m_channelId);

	for (ID pluginId : ch.pluginIds)
		list->addWidget(new gePluginElement(pluginId, m_channelId, 0, 0, 0));
	
	addPlugin = list->addWidget(new geButton(0, 0, 0, G_GUI_UNIT, "-- add new plugin --"));
	
	addPlugin->callback(cb_addPlugin, (void*)this);
}


/* -------------------------------------------------------------------------- */


void gdPluginList::cb_addPlugin()
{
	int wx = m::conf::pluginChooserX;
	int wy = m::conf::pluginChooserY;
	int ww = m::conf::pluginChooserW;
	int wh = m::conf::pluginChooserH;
	u::gui::openSubWindow(G_MainWin, new v::gdPluginChooser(wx, wy, ww, wh, 
		m_channelId), WID_FX_CHOOSER);
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
}} // giada::v::


#endif // #ifdef WITH_VST
