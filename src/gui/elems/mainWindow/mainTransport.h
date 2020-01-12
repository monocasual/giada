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


#ifndef GE_MAIN_TRANSPORT_H
#define GE_MAIN_TRANSPORT_H


#include <FL/Fl_Pack.H>


class geButton;
class geStatusButton;


namespace giada {
namespace v
{
class geMainTransport : public Fl_Pack
{
public:

	geMainTransport(int x, int y);

	void refresh();

private:

	geButton* rewind;
	geStatusButton* play;
	
	geButton* recTriggerMode;
	geStatusButton* recAction;
	geStatusButton* recInput;

	geStatusButton* metronome;
};
}} // giada::v::


#endif
