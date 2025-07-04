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

#ifndef GD_PLUGIN_CHOOSER_H
#define GD_PLUGIN_CHOOSER_H

#include "src/core/types.h"
#include "src/gui/dialogs/window.h"
#include "src/gui/model.h"

namespace giada::v
{
class geTextButton;
class geChoice;
class gePluginBrowser;
class gdPluginChooser : public gdWindow
{
public:
	gdPluginChooser(ID channelId, const Model&);
	~gdPluginChooser();

private:
	m::PluginManager::SortMode getSortMode() const;

	geChoice*        sortMethod;
	geChoice*        sortDir;
	geTextButton*    addBtn;
	geTextButton*    cancelBtn;
	gePluginBrowser* browser;

	ID m_channelId;
};
} // namespace giada::v

#endif