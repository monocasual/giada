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

#include "src/gui/dialogs/about.h"
#include "src/gui/elems/basics/box.h"
#include "src/gui/elems/basics/flex.h"
#include "src/gui/elems/basics/textButton.h"
#include "src/gui/graphics.h"
#include "src/gui/ui.h"
#include "src/utils/gui.h"
#include "src/utils/string.h"
#include <fmt/core.h>

extern giada::v::Ui* g_ui;

namespace giada::v
{
gdAbout::gdAbout()
#ifdef WITH_VST
: gdWindow(u::gui::getCenterWinBounds({-1, -1, 340, 415}), g_ui->getI18Text(LangMap::ABOUT_TITLE), WID_ABOUT)
#else
: gdWindow(u::gui::getCenterWinBounds({-1, -1, 340, 330}), g_ui->getI18Text(LangMap::ABOUT_TITLE), WID_ABOUT)
#endif
{
#ifdef G_DEBUG_MODE
	constexpr bool debug = true;
#else
	constexpr bool debug = false;
#endif

	geFlex* container = new geFlex(getContentBounds().reduced({G_GUI_OUTER_MARGIN}), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		geFlex* body = new geFlex(Direction::VERTICAL);
		{
			geBox* logo = new geBox();
			logo->setSvgImage(graphics::giadaLogo);

			geBox* text = new geBox();
			text->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_TOP);
			text->copy_label(fmt::format(fmt::runtime(g_ui->getI18Text(LangMap::ABOUT_BODY)),
			    G_VERSION.toString(), debug ? "Debug" : "Release", BUILD_DATE)
			                     .c_str());

#ifdef WITH_VST
			geBox* vstLogo = new geBox();
			vstLogo->setSvgImage(graphics::vstLogo);

			geBox* vstText = new geBox();
			vstText->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_TOP);
			vstText->label(g_ui->getI18Text(LangMap::ABOUT_BODY_VST));
#endif

			body->addWidget(logo, 120);
			body->addWidget(text, 140);
#ifdef WITH_VST
			body->addWidget(vstLogo, 60);
			body->addWidget(vstText);
#endif
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

	set_modal();
	show();
}
} // namespace giada::v