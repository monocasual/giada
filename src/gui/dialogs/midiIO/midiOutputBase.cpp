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


#include "core/midiDispatcher.h"
#include "utils/log.h"
#include "utils/string.h"
#include "gui/elems/midiLearner.h"
#include "midiOutputBase.h"


namespace giada {
namespace v 
{
gdMidiOutputBase::gdMidiOutputBase(int w, int h)
	: gdWindow(w, h, "Midi Output Setup")
{
}


/* -------------------------------------------------------------------------- */


gdMidiOutputBase::~gdMidiOutputBase()
{
	m::midiDispatcher::stopMidiLearn();
}


/* -------------------------------------------------------------------------- */


void gdMidiOutputBase::refresh()
{
	for (geMidiLearner* l : m_learners)
		l->refresh();	
}


/* -------------------------------------------------------------------------- */


void gdMidiOutputBase::cb_close(Fl_Widget* w, void* p)  { ((gdMidiOutputBase*)p)->cb_close(); }


/* -------------------------------------------------------------------------- */


void gdMidiOutputBase::cb_close()
{
	do_callback();
}


/* -------------------------------------------------------------------------- */


void gdMidiOutputBase::cb_enableLightning(Fl_Widget* w, void* p)
{
	((gdMidiOutputBase*)p)->cb_enableLightning();
}


/* -------------------------------------------------------------------------- */


void gdMidiOutputBase::cb_enableLightning() {}


/* -------------------------------------------------------------------------- */


void gdMidiOutputBase::setTitle(int chanNum)
{
	std::string tmp = "MIDI Output Setup (channel " + std::to_string(chanNum) + ")"; 
	copy_label(tmp.c_str());
}

}} // giada::v::
