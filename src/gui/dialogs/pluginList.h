/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#ifndef GD_PLUGINLIST_H
#define GD_PLUGINLIST_H


#include "core/pluginHost.h"
#include "window.h"


class geLiquidScroll;
class geButton;


namespace giada {
namespace v
{
class gePluginElement;
class gdPluginList : public gdWindow
{
public:

	gdPluginList(ID chanID);
	~gdPluginList();

	void rebuild() override;

	const gePluginElement& getNextElement(const gePluginElement& curr) const;
	const gePluginElement& getPrevElement(const gePluginElement& curr) const;

private:

	static void cb_addPlugin(Fl_Widget* v, void* p);
	void cb_addPlugin();

	geButton*       addPlugin;
	geLiquidScroll* list;	

	ID m_channelId;
};

}} // giada::v::


#endif

#endif // #ifdef WITH_VST
