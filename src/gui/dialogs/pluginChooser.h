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

#ifndef GD_PLUGIN_CHOOSER_H
#define GD_PLUGIN_CHOOSER_H

#include "core/conf.h"
#include "core/types.h"
#include "window.h"

namespace giada::v
{
class geTextButton;
class geChoice;
class gePluginBrowser;
class gdPluginChooser : public gdWindow
{
public:
	gdPluginChooser(ID channelId, m::Conf&);
	~gdPluginChooser();

private:
	m::Conf& m_conf;

	geChoice*        sortMethod;
	geTextButton*    addBtn;
	geTextButton*    cancelBtn;
	gePluginBrowser* browser;

	ID m_channelId;
};
} // namespace giada::v

#endif