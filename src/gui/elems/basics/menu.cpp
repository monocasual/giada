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

#include "src/gui/elems/basics/menu.h"
#include "src/gui/const.h"
#include "src/gui/elems/basics/boxtypes.h"
#include "src/utils/gui.h"
#include <FL/Fl.H>
#include <cstdint>
#include <cstdio>
#include <span>

namespace giada::v
{
namespace
{
const Fl_Menu_Item* findItemById_(ID targetId, std::span<const Fl_Menu_Item> items)
{
	for (const auto& item : items)
	{
		if (item.flags & FL_SUBMENU_POINTER) // This is a sub-menu
		{
			const Fl_Menu_Item* subMenu     = static_cast<Fl_Menu_Item*>(item.user_data());
			const std::size_t   subMenuSize = subMenu->size();

			const Fl_Menu_Item* found = findItemById_(targetId, {subMenu, subMenuSize});
			if (found != nullptr)
				return found;
		}
		else if (item.label() != nullptr) // This is a regular, valid menu item
		{
			const ID id = static_cast<ID>(reinterpret_cast<intptr_t>(item.user_data()));
			if (id == targetId)
				return &item;
		}
	}
	return nullptr;
}
} // namespace

/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */

geMenu::geMenu()
: Fl_Menu_Button(Fl::event_x(), Fl::event_y(), 0, 0)
, onSelect(nullptr)
{
	box(G_CUSTOM_BORDER_BOX);
	textsize(G_GUI_FONT_SIZE_BASE);
	textcolor(G_COLOR_LIGHT_2);
	color(G_COLOR_GREY_2);
}

/* -------------------------------------------------------------------------- */

void geMenu::callback(Fl_Widget* w, void* p) { static_cast<geMenu*>(w)->callback((ID)(intptr_t)p); }

/* -------------------------------------------------------------------------- */

void geMenu::callback(ID id)
{
	if (onSelect != nullptr)
		onSelect(id);
}

/* -------------------------------------------------------------------------- */

void geMenu::addItem(ID id, const char* text, int flags)
{
	assert(findItemById(id) == nullptr);

	add(text, /*shortcut=*/0, callback, /*data=*/(void*)(intptr_t)(id), flags);
	menu_end();
}

void geMenu::addItem(ID id, const std::string& text, int flags)
{
	addItem(id, text.c_str(), flags);
}

void geMenu::addSubMenu(const std::string& text, geMenu& subMenu)
{
	add(text.c_str(), 0, nullptr, (void*)subMenu.menu(), FL_SUBMENU_POINTER);
}

/* -------------------------------------------------------------------------- */

void geMenu::setEnabled(ID id, bool value)
{
	assert(findItemById(id) != nullptr);

	Fl_Menu_Item& item = *const_cast<Fl_Menu_Item*>(findItemById(id));
	value ? item.activate() : item.deactivate();
}

/* -------------------------------------------------------------------------- */

const Fl_Menu_Item* geMenu::findItemById(ID targetId) const
{
	return findItemById_(targetId, {menu(), static_cast<std::size_t>(size())});
}
} // namespace giada::v
