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

#include "gui/elems/keyBinder.h"
#include "core/const.h"
#include "glue/layout.h"
#include "gui/dialogs/keyGrabber.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/textButton.h"
#include "gui/ui.h"
#include "utils/gui.h"

extern giada::v::Ui g_ui;

namespace giada::v
{
geKeyBinder::geKeyBinder(const std::string& l, int key)
: geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN)
, m_key(key)
{
	m_labelBox = new geBox(l.c_str());
	m_keyBox   = new geBox(u::gui::keyToString(key).c_str());
	m_bindBtn  = new geTextButton(g_ui.getI18Text(LangMap::COMMON_BIND));
	m_clearBtn = new geTextButton(g_ui.getI18Text(LangMap::COMMON_CLEAR));

	add(m_labelBox);
	add(m_keyBox, 100);
	add(m_bindBtn, 50);
	add(m_clearBtn, 50);
	end();

	m_labelBox->box(G_CUSTOM_BORDER_BOX);
	m_keyBox->box(G_CUSTOM_BORDER_BOX);

	m_bindBtn->onClick = [key, this]() {
		c::layout::openKeyGrabberWindow(key, [this](int newKey) {
			m_key = newKey;
			m_keyBox->copy_label(u::gui::keyToString(m_key).c_str());
			return true;
		});
	};

	m_clearBtn->onClick = [key, this]() {
		m_key = 0;
		m_keyBox->copy_label(u::gui::keyToString(key).c_str());
	};
}

/* -------------------------------------------------------------------------- */

int geKeyBinder::getKey() const { return m_key; }
} // namespace giada::v