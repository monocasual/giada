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

#ifndef GE_CHECK_H
#define GE_CHECK_H

#include <FL/Fl_Check_Button.H>
#include <functional>

namespace giada::v
{
class geCheck : public Fl_Check_Button
{
public:
	geCheck(int x, int y, int w, int h, const char* l = nullptr);

	void draw() override;

	std::function<void(bool)> onChange = nullptr;

private:
	static void cb_onChange(Fl_Widget* w, void* p);
	void        cb_onChange();
};
} // namespace giada::v

#endif
