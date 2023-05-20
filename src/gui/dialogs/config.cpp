/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/flex.h"
#include "gui/elems/basics/tabs.h"
#include "gui/elems/basics/textButton.h"
#include "gui/elems/config/tabAudio.h"
#include "gui/elems/config/tabBehaviors.h"
#include "gui/elems/config/tabBindings.h"
#include "gui/elems/config/tabMidi.h"
#include "gui/elems/config/tabMisc.h"
#include "gui/elems/config/tabPlugins.h"
#include "gui/ui.h"
#include "utils/gui.h"

extern giada::v::Ui g_ui;

namespace giada::v
{
gdConfig::gdConfig(int w, int h, const Model& model)
: gdWindow(u::gui::getCenterWinBounds({-1, -1, w, h}), g_ui.getI18Text(LangMap::CONFIG_TITLE))
{
	const geompp::Rect<int> bounds = getContentBounds().reduced(G_GUI_OUTER_MARGIN);

	geFlex* container = new geFlex(bounds, Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		geTabs* tabs = new geTabs(bounds);
		{
			tabAudio     = new geTabAudio(bounds);
			tabMidi      = new geTabMidi(bounds);
			tabBehaviors = new geTabBehaviors(bounds);
			tabMisc      = new geTabMisc(bounds);
			tabBindings  = new geTabBindings(bounds, model);
			tabPlugins   = new geTabPlugins(bounds);

			tabs->add(tabAudio);
			tabs->add(tabMidi);
			tabs->add(tabBehaviors);
			tabs->add(tabPlugins);
			tabs->add(tabBindings);
			tabs->add(tabMisc);
		}

		geFlex* footer = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			geTextButton* closeBtn = new geTextButton(g_ui.getI18Text(LangMap::COMMON_CLOSE));
			closeBtn->onClick      = [this]() { do_callback(); };

			footer->add(new geBox()); // Spacer
			footer->add(closeBtn, 80);
			footer->end();
		}

		container->add(tabs);
		container->add(footer, G_GUI_UNIT);
		container->end();
	}

	add(container);
	resizable(container);
	size_range(w, h);

	setId(WID_CONFIG);
	show();
}
} // namespace giada::v
