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

#include "src/gui/dialogs/missingAssets.h"
#include "src/core/model/loadState.h"
#include "src/gui/const.h"
#include "src/gui/elems/basics/box.h"
#include "src/gui/elems/basics/browser.h"
#include "src/gui/elems/basics/flex.h"
#include "src/gui/elems/basics/textButton.h"
#include "src/gui/ui.h"
#include "src/utils/gui.h"
#include <FL/Fl_Group.H>

extern giada::v::Ui* g_ui;

namespace giada::v
{
gdMissingAssets::gdMissingAssets(const m::model::LoadState& state)
: gdWindow(u::gui::getCenterWinBounds({-1, -1, 400, 300}), g_ui->getI18Text(LangMap::COMMON_WARNING), WID_MISSING_ASSETS)
{
	geFlex* container = new geFlex(getContentBounds().reduced({G_GUI_OUTER_MARGIN}), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		geFlex* body = new geFlex(Direction::VERTICAL, G_GUI_INNER_MARGIN);
		{
			geBox* textIntro = new geBox(g_ui->getI18Text(LangMap::MISSINGASSETS_INTRO), FL_ALIGN_LEFT);
			textIntro->color(G_COLOR_BLUE);

			body->addWidget(textIntro, G_GUI_UNIT);

			if (state.missingWaves.size() > 0)
			{
				geBrowser* waves = new geBrowser();
				for (const std::string& s : state.missingWaves)
					waves->add(s.c_str());
				body->addWidget(new geBox(g_ui->getI18Text(LangMap::MISSINGASSETS_AUDIOFILES), FL_ALIGN_LEFT), G_GUI_UNIT);
				body->addWidget(waves);
			}

			if (state.missingPlugins.size() > 0)
			{
				geBrowser* plugins = new geBrowser();
				for (const std::string& s : state.missingPlugins)
					plugins->add(s.c_str());
				body->addWidget(new geBox(g_ui->getI18Text(LangMap::MISSINGASSETS_PLUGINS), FL_ALIGN_LEFT), G_GUI_UNIT);
				body->addWidget(plugins);
			}
			body->end();
		}

		geFlex* footer = new geFlex(Direction::HORIZONTAL);
		{
			geTextButton* close = new geTextButton(g_ui->getI18Text(LangMap::COMMON_CLOSE));
			close->onClick      = [this]()
			{ do_callback(); };
			footer->addWidget(new geBox()); // Spacer
			footer->addWidget(close, 80);
			footer->end();
		}

		container->addWidget(body);
		container->addWidget(footer, G_GUI_UNIT);
		container->end();
	}

	add(container);
	resizable(container);

	set_modal();
	show();
	wait_for_expose();
}
} // namespace giada::v