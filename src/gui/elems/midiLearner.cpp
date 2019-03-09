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


#include "utils/string.h"
#include "core/midiDispatcher.h"
#include "core/midiEvent.h"
#include "glue/io.h"
#include "gui/dialogs/midiIO/midiInputBase.h"
#include "basics/boxtypes.h"
#include "basics/button.h"
#include "basics/box.h"
#include "midiLearner.h"


namespace giada {
namespace v 
{
geMidiLearner::geMidiLearner(int X, int Y, int W, const char* l, 
	std::atomic<uint32_t>& param, const m::Channel* ch)
: Fl_Group(X, Y, W, 20),
  m_ch    (ch),
  m_param (param)
{
	begin();
	m_text   = new geBox(x(), y(), 156, 20, l);
	m_value  = new geButton(m_text->x()+m_text->w()+4, y(), 80, 20);
	m_button = new geButton(m_value->x()+m_value->w()+4, y(), 40, 20, "learn");
	end();

	m_text->box(G_CUSTOM_BORDER_BOX);
	m_text->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE);

	m_value->box(G_CUSTOM_BORDER_BOX);
	m_value->callback(cb_value, (void*)this);
	m_value->when(FL_WHEN_RELEASE);

	m_button->type(FL_TOGGLE_BUTTON);
	m_button->callback(cb_button, (void*)this);
	
	refresh();
}


/* -------------------------------------------------------------------------- */


void geMidiLearner::refresh()
{
	std::string tmp = "(not set)";
	
	if (m_param != 0x0) {
		tmp = "0x" + u::string::iToString(m_param.load(), true); // true: hex mode
		tmp.pop_back();  // Remove last two digits, useless in MIDI messages
		tmp.pop_back();  // Remove last two digits, useless in MIDI messages
	}

	m_value->copy_label(tmp.c_str());
	m_button->value(0);
}


/* -------------------------------------------------------------------------- */


void geMidiLearner::cb_button(Fl_Widget* v, void* p) { ((geMidiLearner*)p)->cb_button(); }
void geMidiLearner::cb_value(Fl_Widget* v, void* p) { ((geMidiLearner*)p)->cb_value(); }


/* -------------------------------------------------------------------------- */


void geMidiLearner::cb_value()
{
	if (Fl::event_button() == FL_RIGHT_MOUSE)
		c::io::midiLearn(m::MidiEvent(), m_param, m_ch);  // Empty event (0x0)
}


/* -------------------------------------------------------------------------- */


void geMidiLearner::cb_button()
{
	if (m_button->value() == 1)
		m::midiDispatcher::startMidiLearn([this](m::MidiEvent e) 
		{
			c::io::midiLearn(e, m_param, m_ch);
		});
	else
		m::midiDispatcher::stopMidiLearn();	
}

}} // giada::v::
