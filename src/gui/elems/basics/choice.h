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

#ifndef GE_CHOICE_H
#define GE_CHOICE_H

#include "src/gui/elems/basics/box.h"
#include "src/gui/elems/basics/flex.h"
#include <FL/Fl_Choice.H>
#include <functional>
#include <string>
#include <vector>

namespace giada::v
{
/* TODO - changed ID to basic 'int' as the new Id class from mcl::utils, aliased
to giada::ID is not compatible here anymore. Convert it back to mcl::utils::Id
in the future. */
class geChoice : public geFlex
{
public:
	/* geChoice
	Constructors. If label is != nullptr but labelWidth is not specified, the
	label width is automatically computed and adjusted accordingly. */

	geChoice(int x, int y, int w, int h, const char* l = nullptr, int labelWidth = 0);
	geChoice(const char* l = nullptr, int labelWidth = 0);

	int         getSelectedId() const;
	std::string getSelectedLabel() const;
	std::size_t countItems() const;
	bool        hasItem(int id) const;
	bool        hasItem(const std::string&) const;

	/* addItem
	Adds a new item with a certain ID. Pass id = -1 to auto-increment it (ID
	starts from 0). */

	void addItem(const std::string& label, int id = -1);

	/* showFirstItem
	Shows the first, default item. */

	void showFirstItem();

	void showItem(const std::string& label);
	void showItem(int id);
	void activate();
	void deactivate();

	void clear();

	std::function<void(int)> onChange = nullptr;

private:
	class geMenu : public Fl_Choice
	{
	public:
		geMenu(int x, int y, int w, int h);
		void draw() override;
	};

	static void cb_onChange(Fl_Widget* w, void* p);
	void        cb_onChange();

	geBox*           m_text;
	geMenu*          m_menu;
	std::vector<int> m_ids;
};
} // namespace giada::v

#endif
