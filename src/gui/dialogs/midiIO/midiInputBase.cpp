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


#include "core/channels/channel.h"
#include "core/midiDispatcher.h"
#include "core/conf.h"
#include "utils/log.h"
#include "gui/elems/midiLearner.h"
#include "midiInputBase.h"


namespace giada {
namespace v 
{
gdMidiInputBase::gdMidiInputBase(int x, int y, int w, int h, const char* title)
: gdWindow(x, y, w, h, title)
{
}


/* -------------------------------------------------------------------------- */


gdMidiInputBase::~gdMidiInputBase()
{
	m::midiDispatcher::stopMidiLearn();
	
	m::conf::midiInputX = x();
	m::conf::midiInputY = y();
	m::conf::midiInputW = w();
	m::conf::midiInputH = h();
}


/* -------------------------------------------------------------------------- */


void gdMidiInputBase::refresh()
{
	for (geMidiLearner* l : m_learners)
		l->refresh();
}


/* -------------------------------------------------------------------------- */


void gdMidiInputBase::cb_close(Fl_Widget* w, void* p) { ((gdMidiInputBase*)p)->cb_close(); }


/* -------------------------------------------------------------------------- */


void gdMidiInputBase::cb_close()
{
	do_callback();
}

}} // giada::v::
