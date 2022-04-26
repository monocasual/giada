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

#include "core/conf.h"
#include "core/const.h"
#include "core/graphics.h"
#include <FL/Fl_Pixmap.H>
#include <FL/fl_draw.H>
#ifdef WITH_VST
#include "deps/juce-config.h"
#endif
#include "gui/dialogs/about.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/flex.h"
#include "utils/gui.h"
#include "utils/string.h"
#include <fmt/core.h>

namespace giada::v
{
gdAbout::gdAbout()
#ifdef WITH_VST
: gdWindow(340, 415, "About Giada")
#else
: gdWindow(340, 330, "About Giada")
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
			logo->image(new Fl_Pixmap(giada_logo_xpm));

			geBox* text = new geBox();
			text->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_TOP);
			text->copy_label(fmt::format(
			    "Version {} ({} build) {}\n\n"
			    "Developed by Monocasual Laboratories\n\n"
			    "Released under the terms of the GNU General\n"
			    "Public License (GPL v3)\n\n"
			    "News, infos, contacts and documentation:\n"
			    "www.giadamusic.com",
			    G_VERSION_STR, debug ? "Debug" : "Release", BUILD_DATE)
			                     .c_str());

#ifdef WITH_VST
			geBox* vstLogo = new geBox();
			vstLogo->image(new Fl_Pixmap(vstLogo_xpm));
			vstLogo->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);

			geBox* vstText = new geBox();
			vstText->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_TOP);
			vstText->label(
			    "VST Plug-In Technology by Steinberg\n"
			    "VST is a trademark of Steinberg\nMedia Technologies GmbH");
#endif

			body->add(logo, 120);
			body->add(text, 140);
#ifdef WITH_VST
			body->add(vstLogo, 60);
			body->add(vstText);
#endif
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

	set_modal();
	u::gui::setFavicon(this);
	setId(WID_ABOUT);
	show();
}
} // namespace giada::v