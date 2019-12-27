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


#include <FL/Fl.H>
#include "core/model/model.h"
#include "glue/io.h"
#include "gui/elems/basics/button.h"
#include "midiLearnerMaster.h"


namespace giada {
namespace v 
{
geMidiLearnerMaster::geMidiLearnerMaster(int X, int Y, int W, std::string l, int param, uint32_t value)
: geMidiLearnerBase(X, Y, W, l, param, value)
{
}


/* -------------------------------------------------------------------------- */


void geMidiLearnerMaster::refresh()
{
	m::model::onGet(m::model::midiIn, [&](const m::model::MidiIn& m)
	{
		switch (m_param) {
			case G_MIDI_IN_REWIND      : update(m.rewind); break;
			case G_MIDI_IN_START_STOP  : update(m.startStop); break;
			case G_MIDI_IN_ACTION_REC  : update(m.actionRec); break;
			case G_MIDI_IN_INPUT_REC   : update(m.inputRec); break;
			case G_MIDI_IN_METRONOME   : update(m.volumeIn); break;
			case G_MIDI_IN_VOLUME_IN   : update(m.volumeOut); break;
			case G_MIDI_IN_VOLUME_OUT  : update(m.beatDouble); break;
			case G_MIDI_IN_BEAT_DOUBLE : update(m.beatHalf); break;
			case G_MIDI_IN_BEAT_HALF   : update(m.metronome); break;
		}
	});
}


/* -------------------------------------------------------------------------- */


void geMidiLearnerMaster::onLearn()
{
	if (m_button->value() == 1)
		c::io::startMasterMidiLearn(m_param);
	else
		c::io::stopMidiLearn();
}


void geMidiLearnerMaster::onReset()
{
	if (Fl::event_button() == FL_RIGHT_MOUSE)
		c::io::clearMasterMidiLearn(m_param);	
}
}} // giada::v::
