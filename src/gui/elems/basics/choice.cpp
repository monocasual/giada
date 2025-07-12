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

#include "src/gui/elems/basics/choice.h"
#include "src/gui/const.h"
#include "src/gui/drawing.h"
#include "src/utils/gui.h"
#include "src/utils/vector.h"
#include <FL/fl_draw.H>
#include <cassert>

namespace giada::v
{
geChoice::geMenu::geMenu(int x, int y, int w, int h)
: Fl_Choice(x, y, w, h)
{
	labelsize(G_GUI_FONT_SIZE_BASE);
	labelcolor(G_COLOR_LIGHT_2);
	box(FL_BORDER_BOX);
	textsize(G_GUI_FONT_SIZE_BASE);
	textcolor(G_COLOR_LIGHT_2);
	color(G_COLOR_GREY_2);
}

/* -------------------------------------------------------------------------- */

void geChoice::geMenu::draw()
{
	geompp::Rect<int> bounds(x(), y(), w(), h());

	drawRectf(bounds, G_COLOR_GREY_2);                       // background
	drawRect(bounds, static_cast<Fl_Color>(G_COLOR_GREY_4)); // border
	fl_polygon(x() + w() - 8, y() + h() - 1, x() + w() - 1, y() + h() - 8, x() + w() - 1, y() + h() - 1);
	if (value() != -1)
		drawText(u::gui::truncate(text(value()), bounds.w - 16), bounds, FL_HELVETICA,
		    G_GUI_FONT_SIZE_BASE, active() ? G_COLOR_LIGHT_2 : G_COLOR_GREY_4);
}

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geChoice::geChoice(int x, int y, int w, int h, const char* l, int labelWidth)
: geFlex(x, y, w, h, Direction::HORIZONTAL, G_GUI_INNER_MARGIN)
, m_text(nullptr)
, m_menu(nullptr)
{
	if (l != nullptr)
	{
		m_text = new geBox(l, FL_ALIGN_RIGHT);
		addWidget(m_text, labelWidth != 0 ? labelWidth : u::gui::getStringRect(l).w);
	}
	m_menu = new geMenu(x, y, w, h);
	addWidget(m_menu);
	end();
}

/* -------------------------------------------------------------------------- */

geChoice::geChoice(const char* l, int labelWidth)
: geChoice(0, 0, 0, 0, l, labelWidth)
{
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

ID geChoice::getSelectedId() const
{
	return m_menu->value() == -1 ? -1 : m_ids.at(m_menu->value());
}

/* -------------------------------------------------------------------------- */

void geChoice::addItem(const std::string& label, ID id)
{
	m_menu->add(label.c_str(), 0, cb_onChange, static_cast<void*>(this));

	if (id != -1)
		m_ids.push_back(id);
	else // auto-increment
		m_ids.push_back(m_ids.size() == 0 ? 0 : m_ids.back() + 1);
}

/* -------------------------------------------------------------------------- */

void geChoice::showFirstItem()
{
	m_menu->value(0);
}

void geChoice::showItem(const std::string& label)
{
	const int index = m_menu->find_index(label.c_str());
	assert(index != -1);
	m_menu->value(index);
}

void geChoice::showItem(ID id)
{
	const std::size_t index = u::vector::indexOf(m_ids, id);
	assert(index < m_ids.size());
	m_menu->value(index);
}

/* -------------------------------------------------------------------------- */

void geChoice::activate()
{
	geFlex::activate();
	m_menu->activate();
	if (m_text != nullptr)
		m_text->activate();
}

void geChoice::deactivate()
{
	geFlex::deactivate();
	m_menu->deactivate();
	if (m_text != nullptr)
		m_text->deactivate();
}

/* -------------------------------------------------------------------------- */

std::string geChoice::getSelectedLabel() const
{
	return m_menu->text();
}

/* -------------------------------------------------------------------------- */

std::size_t geChoice::countItems() const
{
	return m_ids.size();
}

/* -------------------------------------------------------------------------- */

bool geChoice::hasItem(ID oid) const
{
	return u::vector::has(m_ids, [oid](const ID& id)
	{ return oid == id; });
}

bool geChoice::hasItem(const std::string& label) const
{
	return m_menu->find_index(label.c_str()) != -1;
}

/* -------------------------------------------------------------------------- */

void geChoice::clear()
{
	m_menu->clear();
	m_ids.clear();
}

} // namespace giada::v