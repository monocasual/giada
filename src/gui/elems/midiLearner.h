/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#include <atomic>
#include <FL/Fl_Group.H>


class geBox;
class geButton;


namespace giada {
namespace m
{
class Channel;	
}
namespace v 
{
class geMidiLearner : public Fl_Group
{
public:

	geMidiLearner(int x, int y, int w, const char* l, std::atomic<uint32_t>& param, 
		const m::Channel* ch);

	void refresh();

private:

	static void cb_button(Fl_Widget* v, void* p);
	static void cb_value (Fl_Widget* v, void* p);
	void cb_button();
	void cb_value();

	/* m_ch
	Channel it belongs to. Might be nullptr if the learner comes from the MIDI
	input master window. */

	const m::Channel* m_ch;

	/* m_param
	Reference to ch->midiIn[value]. */

	std::atomic<uint32_t>& m_param;

	geBox*    m_text;
	geButton* m_value;
	geButton* m_button;
};
}} // giada::v::


#endif
