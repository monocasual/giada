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

#ifndef GD_PLUGINLIST_H
#define GD_PLUGINLIST_H

#include "core/conf.h"
#include "glue/plugin.h"
#include "window.h"

namespace giada::v
{
class geTextButton;
class geLiquidScroll;
class gePluginElement;
class gdPluginList : public gdWindow
{
public:
	gdPluginList(ID channelId, m::Conf&);
	~gdPluginList();

	void rebuild() override;

	const gePluginElement& getNextElement(const gePluginElement& curr) const;
	const gePluginElement& getPrevElement(const gePluginElement& curr) const;

private:
	m::Conf& m_conf;

	geTextButton*   addPlugin;
	geLiquidScroll* list;

	ID                 m_channelId;
	c::plugin::Plugins m_plugins;
};
} // namespace giada::v

#endif