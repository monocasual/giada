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
#include "gui/elems/plugin/pluginBrowser.h"
#include "utils/gui.h"

namespace giada::v
{
gdPluginChooser::gdPluginChooser(int X, int Y, int W, int H, ID channelId, m::Conf::Data& c)
: gdWindow(X, Y, W, H, "Available plugins")
, m_conf(c)
, m_channelId(channelId)
{
	/* top area */
	Fl_Group* group_top = new Fl_Group(8, 8, w() - 16, 20);
	sortMethod          = new geChoice(group_top->x() + 45, group_top->y(), 100, 20, "Sort by");
	geBox* b1           = new geBox(sortMethod->x() + sortMethod->w(), group_top->y(), 100, 20); // spacer window border <-> menu
	group_top->resizable(b1);
	group_top->end();

	/* center browser */
	browser = new v::gePluginBrowser(8, 36, w() - 16, h() - 70);

	/* ok/cancel buttons */
	Fl_Group* group_btn = new Fl_Group(8, browser->y() + browser->h() + 8, w() - 16, h() - browser->h() - 16);
	geBox*    b2        = new geBox(8, browser->y() + browser->h(), 100, 20); // spacer window border <-> buttons
	addBtn              = new geButton(w() - 88, group_btn->y(), 80, 20, "Add");
	cancelBtn           = new geButton(addBtn->x() - 88, group_btn->y(), 80, 20, "Cancel");
	group_btn->resizable(b2);
	group_btn->end();

	end();

	sortMethod->add("Name");
	sortMethod->add("Category");
	sortMethod->add("Manufacturer");
	sortMethod->callback(cb_sort, (void*)this);
	sortMethod->value(m_conf.pluginSortMethod);

	addBtn->callback(cb_add, (void*)this);
	addBtn->shortcut(FL_Enter);
	cancelBtn->callback(cb_close, (void*)this);

	resizable(browser);
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
	m_conf.pluginSortMethod = sortMethod->value();
}

/* -------------------------------------------------------------------------- */

void gdPluginChooser::cb_close(Fl_Widget* /*w*/, void* p) { ((gdPluginChooser*)p)->cb_close(); }
void gdPluginChooser::cb_add(Fl_Widget* /*w*/, void* p) { ((gdPluginChooser*)p)->cb_add(); }
void gdPluginChooser::cb_sort(Fl_Widget* /*w*/, void* p) { ((gdPluginChooser*)p)->cb_sort(); }

/* -------------------------------------------------------------------------- */

void gdPluginChooser::cb_close()
{
	do_callback();
}

/* -------------------------------------------------------------------------- */

void gdPluginChooser::cb_sort()
{
	c::plugin::sortPlugins(static_cast<m::PluginManager::SortMethod>(sortMethod->value()));
	browser->refresh();
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
