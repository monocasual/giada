/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#include "choice.h"
#include "core/const.h"
#include "utils/gui.h"
#include "utils/vector.h"
#include <FL/fl_draw.H>
#include <cassert>
#include <string>

namespace giada::v
{
geChoice::geChoice(int x, int y, int w, int h, const char* l, bool ang)
: Fl_Choice(x, y, w, h, l)
, m_angle(ang)
{
	labelsize(G_GUI_FONT_SIZE_BASE);
	labelcolor(G_COLOR_LIGHT_2);
	box(FL_BORDER_BOX);
	textsize(G_GUI_FONT_SIZE_BASE);
	textcolor(G_COLOR_LIGHT_2);
	color(G_COLOR_GREY_2);
}

/* -------------------------------------------------------------------------- */

void geChoice::cb_onChange(Fl_Widget* /*w*/, void* p) { (static_cast<geChoice*>(p))->cb_onChange(); }

/* -------------------------------------------------------------------------- */

void geChoice::cb_onChange()
{
	if (onChange != nullptr)
		onChange(getSelectedId());
}

/* -------------------------------------------------------------------------- */

void geChoice::draw()
{
	fl_rectf(x(), y(), w(), h(), G_COLOR_GREY_2);                       // bg
	fl_rect(x(), y(), w(), h(), static_cast<Fl_Color>(G_COLOR_GREY_4)); // border
	if (m_angle)
		fl_polygon(x() + w() - 8, y() + h() - 1, x() + w() - 1, y() + h() - 8, x() + w() - 1, y() + h() - 1);

	/* pick up the text() from the selected item (value()) and print it in
	 * the box and avoid overflows */

	fl_color(!active() ? G_COLOR_GREY_4 : G_COLOR_LIGHT_2);
	if (value() != -1)
		fl_draw(u::gui::truncate(text(value()), w() - 16).c_str(), x(), y(), w(), h(), FL_ALIGN_CENTER);
}

/* -------------------------------------------------------------------------- */

ID geChoice::getSelectedId() const
{
	return value() == -1 ? -1 : m_ids.at(value());
}

/* -------------------------------------------------------------------------- */

void geChoice::addItem(const std::string& label, ID id)
{
	assert(id >= 0);

	Fl_Choice::add(label.c_str(), 0, cb_onChange, static_cast<void*>(this));
	m_ids.push_back(id);
}

/* -------------------------------------------------------------------------- */

void geChoice::showItem(const std::string& label)
{
	value(find_index(label.c_str()));
}

void geChoice::showItem(ID id)
{
	value(u::vector::indexOf(m_ids, id));
}
} // namespace giada::v