/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * geButton
 * A regular button.
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#ifndef GE_MENU_BUTTON_H
#define GE_MENU_BUTTON_H

#include <FL/Fl_Menu_Button.H>
#include <string>

namespace giada {
namespace v
{

class geMenuButton : public Fl_Menu_Button
{
public:

	geMenuButton(int x, int y, int w, int h, const char* l=nullptr,
						const char* txt=nullptr);
	void draw() override;

	void text(std::string txt);
	std::string text();

protected:

	//Fl_Color bgColor0;   // background not clicked
	//Fl_Color bgColor1;   // background clicked
	//Fl_Color bdColor;    // border
	//Fl_Color txtColor;	 // text

	std::string fg_text;	// A text on the button (different than label)
};

}} // giada::v
#endif
