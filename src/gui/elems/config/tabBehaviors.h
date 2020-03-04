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


#ifndef GE_TAB_BEHAVIORS_H
#define GE_TAB_BEHAVIORS_H


#include <FL/Fl_Group.H>


class geRadio;
class geCheck;


namespace giada {
namespace v
{
class geTabBehaviors : public Fl_Group
{
public:

	geTabBehaviors(int x, int y, int w, int h);

	void save();

	geRadio *chansStopOnSeqHalt_1;
	geRadio *chansStopOnSeqHalt_0;
	geCheck *treatRecsAsLoops;
	geCheck *inputMonitorDefaultOn;


private:

	static void cb_radio_mutex(Fl_Widget* w, void* p);
	void cb_radio_mutex(Fl_Widget* w);
};
}} // giada::v::


#endif
