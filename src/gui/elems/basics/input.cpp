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

#include "input.h"
#include "boxtypes.h"
#include "core/const.h"
#include "utils/gui.h"

namespace giada::v
{
geInput::geInput(int x, int y, int w, int h, const char* l, int labelWidth)
: geFlex(x, y, w, h, Direction::HORIZONTAL, G_GUI_INNER_MARGIN)
{
	if (l != nullptr)
	{
		m_text = new geBox(l, FL_ALIGN_RIGHT);
		add(m_text, labelWidth != 0 ? labelWidth : u::gui::getStringRect(l).w);
	}
	m_input = new Fl_Input(x, y, w, h);
	add(m_input);
	end();

	m_input->box(G_CUSTOM_BORDER_BOX);
	m_input->labelsize(G_GUI_FONT_SIZE_BASE);
	m_input->labelcolor(G_COLOR_LIGHT_2);
	m_input->color(G_COLOR_BLACK);
	m_input->textcolor(G_COLOR_LIGHT_2);
	m_input->cursor_color(G_COLOR_LIGHT_2);
	m_input->selection_color(G_COLOR_GREY_4);
	m_input->textsize(G_GUI_FONT_SIZE_BASE);
	m_input->when(FL_WHEN_RELEASE | FL_WHEN_ENTER_KEY);
	m_input->shortcut(FL_Enter);
	m_input->callback(cb_onChange, this);
}

/* -------------------------------------------------------------------------- */

geInput::geInput(const char* l, int labelWidth)
: geInput(0, 0, 0, 0, l, labelWidth)
{
}

/* -------------------------------------------------------------------------- */

std::string geInput::getValue() const { return m_input->value(); }

void geInput::setType(int t) { m_input->type(t); }
void geInput::setLabelAlign(Fl_Align a) { m_text->align(a); }
void geInput::setValue(const std::string& s) { m_input->value(s.c_str()); }
void geInput::setMaximumSize(int s) { m_input->maximum_size(s); };
void geInput::setReadonly(bool v) { m_input->readonly(v); };
void geInput::setCursorColor(int c) { m_input->cursor_color(c); };

/* -------------------------------------------------------------------------- */

void geInput::cb_onChange(Fl_Widget* /*w*/, void* p) { (static_cast<geInput*>(p))->cb_onChange(); }

/* -------------------------------------------------------------------------- */

void geInput::cb_onChange()
{
	if (onChange != nullptr)
		onChange(m_input->value());
}
} // namespace giada::v