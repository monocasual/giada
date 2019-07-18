/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#ifndef GD_PLUGIN_WINDOW_H
#define GD_PLUGIN_WINDOW_H


#include "window.h"


class geBox;
class geSlider;
class geLiquidScroll;


namespace giada {
namespace m
{
class Plugin;
}
namespace v
{
class gdPluginWindow : public gdWindow
{
public:

	gdPluginWindow(ID pluginId);

	void updateParameter(int index, bool changeSlider=false);
	void updateParameters(bool changeSlider=false);

private:

	int getLabelWidth() const;
	
	ID m_pluginId;
		
	geLiquidScroll* m_list;
};
}} // giada::v::


#endif

#endif // #ifdef WITH_VST
