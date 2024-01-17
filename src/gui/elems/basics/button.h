/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2024 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef GE_BUTTON_H
#define GE_BUTTON_H

#include "deps/geompp/src/rect.hpp"
#include <FL/Fl_Button.H>
#include <functional>

namespace giada::v
{
/* geButton
A base class for any other button in Giada. It doesn't use the traditional
Fl_Button::value() method, allowing for a better 'toggleable' logic. Set or get
the button value with setValue() and getValue() instead. */

class geButton : public Fl_Button
{
public:
	int  value() const                 = delete;
	void value(int)                    = delete;
	void callback(Fl_Callback*)        = delete;
	void callback(Fl_Callback*, void*) = delete;

	int handle(int e) override;

	bool              getValue() const;
	geompp::Rect<int> getBounds() const;

	void setToggleable(bool);

	/* setValue
	Sets the button into a particular graphical state. The state will change 
	when the button is pressed or released. No callback fired. */

	void setValue(bool);

	/* forceValue
	Forces the button into a particular graphical state. The state will never
	change. No callback fired. */

	void forceValue(bool);

	std::function<void()> onClick;

protected:
	geButton(int x, int y, int w, int h, const char* l);
	geButton(const char* l);

	bool m_value;
	bool m_toggleable;
	bool m_forced;
};
} // namespace giada::v

#endif
