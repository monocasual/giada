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

#include "gui/dialogs/missingAssets.h"
#include "core/engine.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/browser.h"
#include "gui/elems/basics/flex.h"
#include "gui/elems/basics/textButton.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include <FL/Fl_Group.H>

extern giada::v::Ui g_ui;

namespace giada::v
{
gdMissingAssets::gdMissingAssets(const m::StorageApi::LoadState& state)
: gdWindow(u::gui::getCenterWinBounds({-1, -1, 400, 300}), g_ui.getI18Text(LangMap::COMMON_WARNING))
{
	geFlex* container = new geFlex(getContentBounds().reduced({G_GUI_OUTER_MARGIN}), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		geFlex* body = new geFlex(Direction::VERTICAL, G_GUI_INNER_MARGIN);
		{
			geBox* textIntro = new geBox(g_ui.getI18Text(LangMap::MISSINGASSETS_INTRO), FL_ALIGN_LEFT);
			textIntro->color(G_COLOR_BLUE);

			body->add(textIntro, G_GUI_UNIT);

			if (state.missingWaves.size() > 0)
			{
				geBrowser* waves = new geBrowser();
				for (const std::string& s : state.missingWaves)
					waves->add(s.c_str());
				body->add(new geBox(g_ui.getI18Text(LangMap::MISSINGASSETS_AUDIOFILES), FL_ALIGN_LEFT), G_GUI_UNIT);
				body->add(waves);
			}

			if (state.missingPlugins.size() > 0)
			{
				geBrowser* plugins = new geBrowser();
				for (const std::string& s : state.missingPlugins)
					plugins->add(s.c_str());
				body->add(new geBox(g_ui.getI18Text(LangMap::MISSINGASSETS_PLUGINS), FL_ALIGN_LEFT), G_GUI_UNIT);
				body->add(plugins);
			}
			body->end();
		}

		geFlex* footer = new geFlex(Direction::HORIZONTAL);
		{
			geTextButton* close = new geTextButton(g_ui.getI18Text(LangMap::COMMON_CLOSE));
			close->onClick      = [this]() { do_callback(); };
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
	show();
}
} // namespace giada::v