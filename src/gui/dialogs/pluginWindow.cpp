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

#include "src/gui/dialogs/pluginWindow.h"
#include "src/glue/plugin.h"
#include "src/gui/const.h"
#include "src/gui/elems/basics/liquidScroll.h"
#include "src/gui/elems/plugin/pluginParameter.h"
#include "src/utils/gui.h"
#include <FL/fl_draw.H>

namespace giada::v
{
gdPluginWindow::gdPluginWindow(const c::plugin::Plugin& plugin, ID wid)
: gdWindow(u::gui::getCenterWinBounds({-1, -1, 450, 156}), "", wid)
, m_plugin(plugin)
{
	set_non_modal();

	begin();

	m_list = new geLiquidScroll(G_GUI_OUTER_MARGIN, G_GUI_OUTER_MARGIN,
	    w() - (G_GUI_OUTER_MARGIN * 2), h() - (G_GUI_OUTER_MARGIN * 2),
	    Direction::VERTICAL);

	m_list->type(Fl_Scroll::VERTICAL_ALWAYS);
	m_list->begin();
	int labelWidth = 100; // TODO
	for (int index : m_plugin.paramIndexes)
	{
		int py = m_list->y() + (index * (G_GUI_UNIT + G_GUI_INNER_MARGIN));
		int pw = m_list->w() - m_list->scrollbar_size() - (G_GUI_OUTER_MARGIN * 3);
		new v::gePluginParameter(m_list->x(), py, pw, labelWidth, c::plugin::getParam(index, m_plugin.getPluginRef(), m_plugin.channelId));
	}
	m_list->end();

	end();

	label(m_plugin.name.c_str());

	size_range(450, (G_GUI_UNIT + (G_GUI_OUTER_MARGIN * 2)));
	resizable(m_list);

	show();
}

/* -------------------------------------------------------------------------- */

void gdPluginWindow::updateParameters(bool changeSlider)
{
	for (int index : m_plugin.paramIndexes)
		static_cast<v::gePluginParameter*>(m_list->child(index))->update(c::plugin::getParam(index, m_plugin.getPluginRef(), m_plugin.channelId), changeSlider);
}
} // namespace giada::v