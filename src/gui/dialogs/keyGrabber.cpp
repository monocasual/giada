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

#include "gui/dialogs/keyGrabber.h"
#include "core/conf.h"
#include "glue/channel.h"
#include "glue/io.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/flex.h"
#include "gui/ui.h"
#include "utils/gui.h"
#include "utils/log.h"
#include "utils/string.h"
#include <cassert>

extern giada::v::Ui g_ui;

namespace giada::v
{
gdKeyGrabber::gdKeyGrabber(int key)
: gdWindow(300, 126, g_ui.langMapper.get(LangMap::KEYGRABBER_TITLE))
, onSetKey(nullptr)
, m_key(key)
{
	geFlex* container = new geFlex(getContentBounds().reduced({G_GUI_OUTER_MARGIN}), Direction::VERTICAL, G_GUI_OUTER_MARGIN);
	{
		m_text = new geBox();

		geFlex* footer = new geFlex(Direction::HORIZONTAL, G_GUI_OUTER_MARGIN);
		{
			m_clear  = new geButton(g_ui.langMapper.get(LangMap::COMMON_CLEAR));
			m_cancel = new geButton(g_ui.langMapper.get(LangMap::COMMON_CLOSE));

			footer->add(new geBox()); // Spacer
			footer->add(m_clear, 80);
			footer->add(m_cancel, 80);
			footer->end();
		}

		container->add(m_text);
		container->add(footer, G_GUI_UNIT);
		container->end();
	}

	add(container);

	m_clear->onClick = [this]() {
		assert(onSetKey != nullptr);

		m_key = 0;
		onSetKey(m_key);
		rebuild();
	};

	m_cancel->onClick = [this]() {
		do_callback();
	};

	rebuild();

	u::gui::setFavicon(this);
	set_modal();
	show();
}

/* -------------------------------------------------------------------------- */

void gdKeyGrabber::rebuild()
{
	std::string tmp = g_ui.langMapper.get(LangMap::KEYGRABBER_BODY) + u::gui::keyToString(m_key);
	m_text->copy_label(tmp.c_str());
}

/* -------------------------------------------------------------------------- */

int gdKeyGrabber::handle(int e)
{
	if (e != FL_KEYUP)
		return Fl_Group::handle(e);

	assert(onSetKey != nullptr);

	const int newKey = Fl::event_key();

	if (!onSetKey(newKey))
	{
		u::log::print("Invalid key\n");
		return 1;
	}

	m_key = newKey;
	rebuild();

	u::log::print("Set key '%c' (%d)\n", m_key, m_key);

	return 1;
}
} // namespace giada::v
