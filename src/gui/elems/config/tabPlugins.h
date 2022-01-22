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

#ifndef GE_TAB_PLUGINS_H
#define GE_TAB_PLUGINS_H

#ifdef WITH_VST

#include "glue/config.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/input.h"
#include <FL/Fl_Group.H>

namespace giada::v
{
class geTabPlugins : public Fl_Group
{
public:
	geTabPlugins(int x, int y, int w, int h);

	void save();
	void rebuild();

private:
	static void cb_scan(Fl_Widget* /*w*/, void* p);
	static void cb_browse(Fl_Widget* /*w*/, void* p);
	void        cb_scan();
	void        cb_browse();

	c::config::PluginData m_data;

	geButton m_browse;
	geInput  m_folderPath;
	geButton m_scanButton;
	geBox    m_info;
};
} // namespace giada::v

#endif // WITH_VST

#endif
