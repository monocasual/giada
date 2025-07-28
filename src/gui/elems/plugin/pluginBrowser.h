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

#ifndef GE_PLUGIN_BROWSER_H
#define GE_PLUGIN_BROWSER_H

#include "src/gui/elems/basics/tableText.h"
#include "src/types.h"

namespace giada::v
{
class gePluginBrowser : public geTableText
{
public:
	enum class Column : int
	{
		NAME = 0,
		MANUFACTURER_NAME,
		CATEGORY,
		FORMAT,
		JUCE_ID
	};

	gePluginBrowser();

	std::string getCellText(int row, int col) const override;
	std::string getHeaderText(int col) const override;

	PluginSortMethod getSortMethodByColumn(int col) const;
	std::string      getJuceId(int row) const;

	void rebuild();

private:
	int getContentWidth(int row, int column) const override;

	void prepareLayout();

	std::vector<PluginInfo> m_pluginInfo;
};
} // namespace giada::v

#endif