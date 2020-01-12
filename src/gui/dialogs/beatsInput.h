/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
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


#ifndef GD_BEATSINPUT_H
#define GD_BEATSINPUT_H


#include "window.h"


class geInput;
class geButton;
class geCheck;


namespace giada {
namespace v 
{
class gdBeatsInput : public gdWindow
{
public:

    gdBeatsInput();
    ~gdBeatsInput();

private:

	static void cb_update(Fl_Widget* w, void* p);
	void cb_update();

	geInput* beats;
	geInput* bars;
	geButton* ok;
};
}} // giada::v::


#endif
