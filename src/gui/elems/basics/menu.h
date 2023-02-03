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

#ifndef GE_MENU_H
#define GE_MENU_H

#include "core/types.h"
#include <FL/Fl_Menu_.H>
#include <FL/Fl_Menu_Button.H>
#include <functional>
#include <string>

namespace giada::v
{
class geMenu : public Fl_Menu_Button
{
public:
	geMenu();

	void addItem(ID, const char* text, int flags = 0);
	void setEnabled(ID, bool);

	std::function<void(ID)> onSelect;

private:
	static void callback(Fl_Widget*, void*);
	void        callback(ID);
};
} // namespace giada::v

#endif
