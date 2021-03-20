/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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

#ifndef GE_MAIN_TIMER_H
#define GE_MAIN_TIMER_H

#include "glue/main.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/choice.h"
#include "gui/elems/basics/pack.h"

class geButton;

namespace giada::v
{
class geChoice;
class geMainTimer : public gePack
{
public:
	geMainTimer(int x, int y);

	void refresh();
	void rebuild();

	void setBpm(const char* v);
	void setBpm(float v);
	void setMeter(int beats, int bars);
	void setQuantizer(int q);

	/* setLock
	Locks bpm, meter and multipliers. Used during audio recordings. */

	void setLock(bool v);

  private:
	static void cb_bpm(Fl_Widget* /*w*/, void* p);
	static void cb_meter(Fl_Widget* /*w*/, void* p);
	static void cb_quantizer(Fl_Widget* /*w*/, void* p);
	static void cb_multiplier(Fl_Widget* /*w*/, void* p);
	static void cb_divider(Fl_Widget* /*w*/, void* p);
	void        cb_bpm();
	void        cb_meter();
	void        cb_quantizer();
	void        cb_multiplier();
	void        cb_divider();

	c::main::Timer m_timer;

	geButton m_bpm;
	geButton m_meter;
	geChoice m_quantizer;
	geButton m_multiplier;
	geButton m_divider;
};
} // namespace giada::v

#endif
