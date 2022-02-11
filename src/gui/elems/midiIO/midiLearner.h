/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2022 Giovanni A. Zuliani | Monocasual Laboratories
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

#ifndef GE_MIDI_LEARNER_H
#define GE_MIDI_LEARNER_H

#include "gui/elems/basics/box.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/pack.h"
#include <functional>
#include <string>

class geBox;

namespace giada
{
namespace v
{
class geButton;
class geMidiLearner : public gePack
{
public:
	geMidiLearner(int x, int y, std::string l, int param);

	/* update
	Updates and repaints the label widget with value 'value'. */

	void update(uint32_t value);

	void activate();
	void deactivate();

	std::function<void(uint32_t)> onStartLearn;
	std::function<void()>         onStopLearn;
	std::function<void(uint32_t)> onClearLearn;

protected:
	/* m_param
	Parameter index to be learnt. */

	int m_param;

	geBox    m_text;
	geButton m_valueBtn;
	geButton m_button;

private:
	static void cb_button(Fl_Widget* /*w*/, void* p);
	static void cb_value(Fl_Widget* /*w*/, void* p);

	void onLearn() const;
	void onReset() const;
};
} // namespace v
} // namespace giada

#endif
