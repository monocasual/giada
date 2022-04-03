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

#include "pluginChooser.h"
#include "core/conf.h"
#include "glue/plugin.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/choice.h"
#include "gui/elems/basics/flex.h"
#include "gui/elems/plugin/pluginBrowser.h"
#include "gui/ui.h"
#include "utils/gui.h"

extern giada::v::Ui g_ui;

namespace giada::v
{
gdPluginChooser::gdPluginChooser(int X, int Y, int W, int H, ID channelId, m::Conf::Data& c)
: gdWindow(X, Y, W, H, g_ui.langMapper.get(LangMap::PLUGINCHOOSER_TITLE))
, m_conf(c)
, m_channelId(channelId)
{
	geFlex* container = new geFlex(getContentBounds().reduced({G_GUI_OUTER_MARGIN}), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		geFlex* header = new geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN);
		{
			sortMethod = new geChoice(g_ui.langMapper.get(LangMap::PLUGINCHOOSER_SORTBY), 0);
			header->add(sortMethod, 180);
			header->add(new geBox());
			header->end();
		}

		browser = new v::gePluginBrowser(0, 0, 0, 0);

		geFlex* footer = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			addBtn    = new geButton(g_ui.langMapper.get(LangMap::COMMON_ADD));
			cancelBtn = new geButton(g_ui.langMapper.get(LangMap::COMMON_CANCEL));
			footer->add(new geBox());
			footer->add(cancelBtn, 80);
			footer->add(addBtn, 80);
			footer->end();
		}

		container->add(header, G_GUI_UNIT);
		container->add(browser);
		container->add(footer, G_GUI_UNIT);
		container->end();
	}

	add(container);
	resizable(container);

	sortMethod->addItem(g_ui.langMapper.get(LangMap::PLUGINCHOOSER_SORTBY_NAME));
	sortMethod->addItem(g_ui.langMapper.get(LangMap::PLUGINCHOOSER_SORTBY_CATEGORY));
	sortMethod->addItem(g_ui.langMapper.get(LangMap::PLUGINCHOOSER_SORTBY_MANIFACTURER));
	sortMethod->addItem(g_ui.langMapper.get(LangMap::PLUGINCHOOSER_SORTBY_FORMAT));
	sortMethod->showItem(m_conf.pluginSortMethod);
	sortMethod->onChange = [this](ID id) {
		c::plugin::sortPlugins(static_cast<m::PluginManager::SortMethod>(id));
		browser->refresh();
	};

	addBtn->callback(cb_add, (void*)this);
	addBtn->shortcut(FL_Enter);
	cancelBtn->callback(cb_close, (void*)this);

	u::gui::setFavicon(this);
	show();
}

/* -------------------------------------------------------------------------- */

gdPluginChooser::~gdPluginChooser()
{
	m_conf.pluginChooserX   = x();
	m_conf.pluginChooserY   = y();
	m_conf.pluginChooserW   = w();
	m_conf.pluginChooserH   = h();
	m_conf.pluginSortMethod = sortMethod->getSelectedId();
}

/* -------------------------------------------------------------------------- */

void gdPluginChooser::cb_close(Fl_Widget* /*w*/, void* p) { ((gdPluginChooser*)p)->cb_close(); }
void gdPluginChooser::cb_add(Fl_Widget* /*w*/, void* p) { ((gdPluginChooser*)p)->cb_add(); }

/* -------------------------------------------------------------------------- */

void gdPluginChooser::cb_close()
{
	do_callback();
}

/* -------------------------------------------------------------------------- */

void gdPluginChooser::cb_add()
{
	int pluginIndex = browser->value() - 3; // subtract header lines
	if (pluginIndex < 0)
		return;
	c::plugin::addPlugin(pluginIndex, m_channelId);
	do_callback();
}
} // namespace giada::v

#endif // #ifdef WITH_VST
