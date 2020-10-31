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


#include <cassert>
#include "utils/string.h"
#include "gui/elems/basics/boxtypes.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/button.h"
#include "midiLearner.h"


namespace giada {
namespace v 
{
geMidiLearner::geMidiLearner(int x, int y, std::string l, int param)
: gePack      (x, y, Direction::HORIZONTAL)
, onStartLearn(nullptr)
, onStopLearn (nullptr)
, onClearLearn(nullptr)
, m_param     (param)
, m_text      (0, 0, 146, 20, l.c_str())
, m_valueBtn  (0, 0, 80, 20)
, m_button    (0, 0, 50, 20, "learn")
{
	add(&m_text);
	add(&m_valueBtn);
	add(&m_button);

	m_text.box(G_CUSTOM_BORDER_BOX);
	m_text.align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

	m_valueBtn.box(G_CUSTOM_BORDER_BOX);
	m_valueBtn.callback(cb_value, (void*)this);
	m_valueBtn.when(FL_WHEN_RELEASE);

	m_button.type(FL_TOGGLE_BUTTON);
	m_button.callback(cb_button, (void*)this);
}


/* -------------------------------------------------------------------------- */


void geMidiLearner::cb_button(Fl_Widget* /*w*/, void* p) { ((geMidiLearner*)p)->onLearn(); }
void geMidiLearner::cb_value(Fl_Widget* /*w*/, void* p)  { ((geMidiLearner*)p)->onReset(); }


/* -------------------------------------------------------------------------- */


void geMidiLearner::update(uint32_t value)
{
	std::string tmp = "(not set)";
	
	if (value != 0x0) {
		tmp = "0x" + u::string::iToString(value, /*hex=*/true);
		tmp.pop_back();  // Remove last two digits, useless in MIDI messages
		tmp.pop_back();  // Remove last two digits, useless in MIDI messages
	}

	m_valueBtn.copy_label(tmp.c_str());
	m_button.value(0);	
}


/* -------------------------------------------------------------------------- */


void geMidiLearner::activate()
{
	Fl_Group::activate();
	m_valueBtn.activate();
	m_button.activate();
}


void geMidiLearner::deactivate()
{
	Fl_Group::deactivate();
	m_valueBtn.deactivate();
	m_button.deactivate();
}


/* -------------------------------------------------------------------------- */


void geMidiLearner::onLearn() const
{
	assert(onStartLearn != nullptr);
	assert(onStopLearn != nullptr);

	if (m_button.value() == 1)
		onStartLearn(m_param);
	else
		onStopLearn();
}


/* -------------------------------------------------------------------------- */


void geMidiLearner::onReset() const
{
	assert(onClearLearn != nullptr);

	if (Fl::event_button() == FL_RIGHT_MOUSE)
		onClearLearn(m_param);	
}
}} // giada::v::
