/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef GD_CONFIG_H
#define GD_CONFIG_H

#include "gui/model.h"
#include "window.h"

namespace giada::v
{
class geTabAudio;
class geTabBehaviors;
class geTabMidi;
class geTabMisc;
class geTabBindings;
class geTabPlugins;
class gdConfig : public gdWindow
{
public:
	gdConfig(int w, int h, const Model&);

	geTabAudio*     tabAudio;
	geTabBehaviors* tabBehaviors;
	geTabMidi*      tabMidi;
	geTabMisc*      tabMisc;
	geTabBindings*  tabBindings;
	geTabPlugins*   tabPlugins;
};
} // namespace giada::v

#endif
