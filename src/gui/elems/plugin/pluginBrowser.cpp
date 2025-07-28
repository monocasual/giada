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
gePluginBrowser::gePluginBrowser()
: geTableText()
, m_pluginInfo(c::plugin::getPluginsInfo())
{
	prepareLayout();
}

/* -------------------------------------------------------------------------- */

std::string gePluginBrowser::getCellText(int row, int col) const
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

std::string gePluginBrowser::getHeaderText(int col) const
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

PluginSortMethod gePluginBrowser::getSortMethodByColumn(int col) const
{
	switch (static_cast<gePluginBrowser::Column>(col))
	{
	case gePluginBrowser::Column::NAME:
	default:
		return PluginSortMethod::NAME;
	case gePluginBrowser::Column::MANUFACTURER_NAME:
		return PluginSortMethod::MANUFACTURER;
	case gePluginBrowser::Column::CATEGORY:
		return PluginSortMethod::CATEGORY;
	}
}

/* -------------------------------------------------------------------------- */

std::string gePluginBrowser::getJuceId(int row) const
{
	return getCellText(row, static_cast<int>(Column::JUCE_ID));
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

void gePluginBrowser::prepareLayout()
{
	rows(m_pluginInfo.size());
	row_header(false);
	row_height_all(G_GUI_UNIT);
	row_resize(false);

	cols(5);
	col_header(true);
	col_resize(true);

	if (g_ui->model.pluginChooserLayout[0] == -1)
		fitContent();
	else
		loadLayout(g_ui->model.pluginChooserLayout);
}

/* -------------------------------------------------------------------------- */

int gePluginBrowser::getContentWidth(int row, int column) const
{
	const auto columnToField = [this](int row, int column) -> const std::string&
	{
		switch (static_cast<Column>(column))
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
		}
	};

	return u::gui::getStringRect(columnToField(row, column)).w + G_GUI_OUTER_MARGIN;
}
} // namespace giada::v