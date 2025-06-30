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

#include "gui/dialogs/config.h"
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

extern giada::v::Ui* g_ui;

namespace giada::v
{
gdConfig::gdConfig(const Model& model)
: gdWindow(u::gui::getCenterWinBounds(model.settingsBounds), g_ui->getI18Text(LangMap::CONFIG_TITLE), WID_CONFIG)
{
	const int minW = 420;
	const int minH = 370;

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
			geTextButton* closeBtn = new geTextButton(g_ui->getI18Text(LangMap::COMMON_CLOSE));
			closeBtn->onClick      = [this]()
			{ do_callback(); };

			footer->addWidget(new geBox()); // Spacer
			footer->addWidget(closeBtn, 80);
			footer->end();
		}

		container->addWidget(tabs);
		container->addWidget(footer, G_GUI_UNIT);
		container->end();
	}

	add(container);
	resizable(container);
	size_range(minW, minH);

	show();
}

/* -------------------------------------------------------------------------- */

gdConfig::~gdConfig()
{
	g_ui->model.settingsBounds = getBounds();
}
} // namespace giada::v
