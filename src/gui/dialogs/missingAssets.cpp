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

#include "gui/dialogs/missingAssets.h"
#include "core/engine.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/browser.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/flex.h"
#include "utils/gui.h"
#include <FL/Fl_Group.H>

namespace giada::v
{
gdMissingAssets::gdMissingAssets(const m::LoadState& state)
: gdWindow(u::gui::getCenterWinBounds(400, 300), "Warning")
{
	geFlex* container = new geFlex(getContentBounds().reduced({G_GUI_OUTER_MARGIN}), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		geFlex* body = new geFlex(Direction::VERTICAL, G_GUI_INNER_MARGIN);
		{
			geBox* textIntro = new geBox("This project contains missing assets.", FL_ALIGN_LEFT);
			textIntro->color(G_COLOR_BLUE);

			body->add(textIntro, G_GUI_UNIT);

			if (state.missingWaves.size() > 0)
			{
				geBrowser* waves = new geBrowser();
				for (const std::string& s : state.missingWaves)
					waves->add(s.c_str());
				body->add(new geBox("Audio files not found in the project folder:", FL_ALIGN_LEFT), G_GUI_UNIT);
				body->add(waves);
			}

			if (state.missingPlugins.size() > 0)
			{
				geBrowser* plugins = new geBrowser();
				for (const std::string& s : state.missingPlugins)
					plugins->add(s.c_str());
				body->add(new geBox("Audio plug-ins not found globally:", FL_ALIGN_LEFT), G_GUI_UNIT);
				body->add(plugins);
			}
			body->end();
		}

		geFlex* footer = new geFlex(Direction::HORIZONTAL);
		{
			geButton* close = new geButton("Close");
			close->onClick  = [this]() { do_callback(); };
			footer->add(new geBox()); // Spacer
			footer->add(close, 80);
			footer->end();
		}

		container->add(body);
		container->add(footer, G_GUI_UNIT);
		container->end();
	}

	add(container);
	resizable(container);

	set_modal();
	u::gui::setFavicon(this);
	show();
}
} // namespace giada::v