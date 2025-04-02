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

#include "src/gui/elems/plugin/pluginBrowser.h"
#include "src/glue/plugin.h"
#include "src/gui/const.h"
#include "src/gui/elems/basics/boxtypes.h"
#include "src/gui/ui.h"
#include "src/utils/gui.h"
#include <fmt/core.h>

extern giada::v::Ui* g_ui;

namespace giada::v
{
namespace
{
enum class Column : int
{
	NAME = 0,
	MANUFACTURER_NAME,
	CATEGORY,
	FORMAT,
	JUCE_ID
};
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

gePluginBrowser::gePluginBrowser()
: geTableText()
, m_widths{0}
, m_pluginInfo(c::plugin::getPluginsInfo())
{
	prepareLayout();
}

/* -------------------------------------------------------------------------- */

std::string gePluginBrowser::setCellText(int row, int col)
{
	switch (static_cast<Column>(col))
	{
	case Column::NAME:
		return m_pluginInfo[row].name;
	case Column::MANUFACTURER_NAME:
		return m_pluginInfo[row].manufacturerName;
	case Column::CATEGORY:
		return m_pluginInfo[row].category;
	case Column::FORMAT:
		return m_pluginInfo[row].format;
	case Column::JUCE_ID:
		return m_pluginInfo[row].juceId;
	default:
		return "";
	}

	return "";
}

/* -------------------------------------------------------------------------- */

std::string gePluginBrowser::setHeaderText(int col)
{
	switch (static_cast<Column>(col))
	{
	case Column::NAME:
		return "Name";
	case Column::MANUFACTURER_NAME:
		return "Manufacturer";
	case Column::CATEGORY:
		return "Category";
	case Column::FORMAT:
		return "Format";
	case Column::JUCE_ID:
		return "ID";
	default:
		return "";
	}
}

/* -------------------------------------------------------------------------- */

void gePluginBrowser::rebuild()
{
	m_pluginInfo = c::plugin::getPluginsInfo();

	clear();
	prepareLayout();
	redraw();
}

/* -------------------------------------------------------------------------- */

void gePluginBrowser::computeColumnWidths()
{
	col_width(static_cast<int>(Column::NAME), getLongestString(&m::PluginManager::PluginInfo::name) + G_GUI_OUTER_MARGIN);
	col_width(static_cast<int>(Column::MANUFACTURER_NAME), getLongestString(&m::PluginManager::PluginInfo::manufacturerName) + G_GUI_OUTER_MARGIN);
	col_width(static_cast<int>(Column::CATEGORY), getLongestString(&m::PluginManager::PluginInfo::category) + G_GUI_OUTER_MARGIN);
	col_width(static_cast<int>(Column::FORMAT), getLongestString(&m::PluginManager::PluginInfo::format) + G_GUI_OUTER_MARGIN);
	col_width(static_cast<int>(Column::JUCE_ID), getLongestString(&m::PluginManager::PluginInfo::juceId) + G_GUI_OUTER_MARGIN);
}

/* -------------------------------------------------------------------------- */

void gePluginBrowser::prepareLayout()
{
	rows(m_pluginInfo.size());
	row_header(false);
	row_height_all(G_GUI_UNIT);
	row_resize(false);

	cols(5);
	col_header(true);
	col_resize(true);

	computeColumnWidths();
}

/* -------------------------------------------------------------------------- */

int gePluginBrowser::getLongestString(const std::string m::PluginManager::PluginInfo::* member) const
{
	const auto view = m_pluginInfo | std::ranges::views::transform([member](const m::PluginManager::PluginInfo& i)
	{ return i.*member; });

	const auto longestIt = std::ranges::max_element(view, [](const std::string& a, const std::string& b)
	{
		return a.size() < b.size();
	});

	return u::gui::getStringRect(*longestIt).w;
}
} // namespace giada::v