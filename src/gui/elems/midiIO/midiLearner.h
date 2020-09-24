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


#ifndef GE_MIDI_LEARNER_H
#define GE_MIDI_LEARNER_H


#include <functional>
#include <string>
#include <FL/Fl_Group.H>


class geBox;
class geButton;


namespace giada {
namespace v 
{
class geMidiLearner : public Fl_Group
{
public:

	geMidiLearner(int x, int y, int w, std::string l, int param);

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

	geBox*    m_text;
	geButton* m_valueBtn;
	geButton* m_button;

private:

	static void cb_button(Fl_Widget* /*w*/, void* p);
	static void cb_value (Fl_Widget* /*w*/, void* p);

	void onLearn() const;
	void onReset() const;

};
}} // giada::v::


#endif
