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

#include "gui/dialogs/config.h"
#include "core/conf.h"
#include "core/const.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/flex.h"
#include "gui/elems/config/tabAudio.h"
#include "gui/elems/config/tabBehaviors.h"
#include "gui/elems/config/tabMidi.h"
#include "gui/elems/config/tabMisc.h"
#include "gui/elems/config/tabPlugins.h"
#include "utils/gui.h"
#include <FL/Fl_Tabs.H>

namespace giada::v
{
gdConfig::gdConfig(int w, int h, m::Conf::Data& conf)
: gdWindow(u::gui::getCenterWinBounds(w, h), "Configuration")
{
	geFlex* container = new geFlex(getContentBounds().reduced(G_GUI_OUTER_MARGIN), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		Fl_Tabs* tabs = new Fl_Tabs(8, 8, w - 16, h - 44);
		tabs->box(G_CUSTOM_BORDER_BOX);
		tabs->labelcolor(G_COLOR_LIGHT_2);
		tabs->begin();
		{
			tabAudio     = new geTabAudio(tabs->x() + 10, tabs->y() + 20, tabs->w() - 20, tabs->h() - 40);
			tabMidi      = new geTabMidi(tabs->x() + 10, tabs->y() + 20, tabs->w() - 20, tabs->h() - 40);
			tabBehaviors = new geTabBehaviors(tabs->x() + 10, tabs->y() + 20, tabs->w() - 20, tabs->h() - 40, conf);
			tabMisc      = new geTabMisc(tabs->x() + 10, tabs->y() + 20, tabs->w() - 20);
#ifdef WITH_VST
			tabPlugins = new geTabPlugins(tabs->x() + 10, tabs->y() + 20, tabs->w() - 20, tabs->h() - 40);
#endif
		}
		tabs->end();

		geFlex* footer = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			geButton* save   = new geButton("Save");
			geButton* cancel = new geButton("Cancel");
			save->onClick    = [this]() { saveConfig(); };
			cancel->onClick  = [this]() { do_callback(); };

			footer->add(new geBox()); // Spacer
			footer->add(cancel, 80);
			footer->add(save, 80);
			footer->end();
		}

		container->add(tabs);
		container->add(footer, G_GUI_UNIT);
		container->end();
	}

	add(container);

	u::gui::setFavicon(this);
	setId(WID_CONFIG);
	show();
}

/* -------------------------------------------------------------------------- */

void gdConfig::saveConfig()
{
	tabAudio->save();
	tabBehaviors->save();
	tabMidi->save();
	tabMisc->save();
#ifdef WITH_VST
	tabPlugins->save();
#endif
	do_callback();
}
} // namespace giada::v
