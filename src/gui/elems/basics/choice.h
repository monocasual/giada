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

#ifndef GE_CHOICE_H
#define GE_CHOICE_H

#include "core/types.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/flex.h"
#include <FL/Fl_Choice.H>
#include <functional>
#include <string>
#include <vector>

namespace giada::v
{
class geChoice : public geFlex
{
public:
	/* geChoice 
	Constructors. If label is != nullptr but labelWidth is not specified, the
	label width is automatically computed and adjusted accordingly. */

	geChoice(int x, int y, int w, int h, const char* l = nullptr, int labelWidth = 0);
	geChoice(const char* l = nullptr, int labelWidth = 0);

	ID          getSelectedId() const;
	std::string getSelectedLabel() const;
	std::size_t countItems() const;

	/* addItem
	Adds a new item with a certain ID. Pass id = -1 to auto-increment it (ID
	starts from 0). */

	void addItem(const std::string& label, ID id = -1);

	void showItem(const std::string& label);
	void showItem(ID);
	void activate();
	void deactivate();

	void clear();

	std::function<void(ID)> onChange = nullptr;

private:
	class geMenu : public Fl_Choice
	{
	public:
		geMenu(int x, int y, int w, int h);
		void draw() override;
	};

	static void cb_onChange(Fl_Widget* w, void* p);
	void        cb_onChange();

	geBox*          m_text;
	geMenu*         m_menu;
	std::vector<ID> m_ids;
};
} // namespace giada::v

#endif
