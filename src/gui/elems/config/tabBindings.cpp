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

#include "gui/elems/config/tabBindings.h"
#include "core/const.h"
#include "gui/elems/basics/liquidScroll.h"
#include "gui/elems/keyBinder.h"
#include "gui/ui.h"
#include "utils/gui.h"

extern giada::v::Ui g_ui;

namespace giada::v
{
geTabBindings::geTabBindings(geompp::Rect<int> bounds, m::Conf& conf)
: Fl_Group(bounds.x, bounds.y, bounds.w, bounds.h, g_ui.getI18Text(LangMap::CONFIG_BINDINGS_TITLE))
{
	end();

	geFlex* body = new geFlex(bounds.reduced(G_GUI_OUTER_MARGIN), Direction::VERTICAL, G_GUI_INNER_MARGIN);
	{
		play          = new geKeyBinder(g_ui.getI18Text(LangMap::CONFIG_BINDINGS_PLAY), conf.keyBindings[m::Conf::KEY_BIND_PLAY]);
		rewind        = new geKeyBinder(g_ui.getI18Text(LangMap::CONFIG_BINDINGS_REWIND), conf.keyBindings[m::Conf::KEY_BIND_REWIND]);
		recordActions = new geKeyBinder(g_ui.getI18Text(LangMap::CONFIG_BINDINGS_RECORDACTIONS), conf.keyBindings[m::Conf::KEY_BIND_RECORD_ACTIONS]);
		recordInput   = new geKeyBinder(g_ui.getI18Text(LangMap::CONFIG_BINDINGS_RECORDAUDIO), conf.keyBindings[m::Conf::KEY_BIND_RECORD_INPUT]);
		exit          = new geKeyBinder(g_ui.getI18Text(LangMap::CONFIG_BINDINGS_EXIT), conf.keyBindings[m::Conf::KEY_BIND_EXIT]);

		body->add(play, G_GUI_UNIT);
		body->add(rewind, G_GUI_UNIT);
		body->add(recordActions, G_GUI_UNIT);
		body->add(recordInput, G_GUI_UNIT);
		body->add(exit, G_GUI_UNIT);
		body->end();
	}

	add(body);
	resizable(body);
}
} // namespace giada::v