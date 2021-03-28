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

#include "mainTransport.h"
#include "core/clock.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/graphics.h"
#include "core/mixer.h"
#include "core/mixerHandler.h"
#include "core/recManager.h"
#include "core/sequencer.h"
#include "glue/events.h"
#include "glue/main.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/statusButton.h"

namespace giada::v
{
geMainTransport::geMainTransport(int x, int y)
: gePack(x, y, Direction::HORIZONTAL)
, m_rewind(0, 0, 25, 25, "", rewindOff_xpm, rewindOn_xpm)
, m_play(0, 0, 25, 25, play_xpm, pause_xpm)
, m_spacer1(0, 0, 10, 25)
, m_recTriggerMode(0, 0, 15, 25, recTriggerModeOff_xpm, recTriggerModeOn_xpm)
, m_recAction(0, 0, 25, 25, recOff_xpm, recOn_xpm)
, m_recInput(0, 0, 25, 25, inputRecOff_xpm, inputRecOn_xpm)
, m_inputRecMode(0, 0, 15, 25, freeInputRecOff_xpm, freeInputRecOn_xpm)
, m_spacer2(0, 0, 10, 25)
, m_metronome(0, 0, 15, 25, metronomeOff_xpm, metronomeOn_xpm)
{
	add(&m_rewind);
	add(&m_play);
	add(&m_spacer1);
	add(&m_recTriggerMode);
	add(&m_recAction);
	add(&m_recInput);
	add(&m_inputRecMode);
	add(&m_spacer2);
	add(&m_metronome);

	m_rewind.copy_tooltip("Rewind");
	m_play.copy_tooltip("Play/Stop");
	m_recTriggerMode.copy_tooltip("Record-on-signal mode\n\nIf enabled, action "
	                              "and audio recording will start only when a signal (key press or audio) "
	                              "is detected.");
	m_recAction.copy_tooltip("Record actions");
	m_recInput.copy_tooltip("Record audio");
	m_inputRecMode.copy_tooltip("Free loop-length mode\n\nIf enabled, the sequencer "
	                            "will adjust to the length of your first audio recording. "
	                            "Available only if there are no other audio samples in the "
	                            "project.");
	m_metronome.copy_tooltip("Metronome");

	m_rewind.callback([](Fl_Widget* /*w*/, void* /*v*/) {
		c::events::rewindSequencer(Thread::MAIN);
	});

	m_play.callback([](Fl_Widget* /*w*/, void* /*v*/) {
		c::events::toggleSequencer(Thread::MAIN);
	});

	m_recAction.callback([](Fl_Widget* /*w*/, void* /*v*/) {
		c::events::toggleActionRecording();
	});

	m_recInput.callback([](Fl_Widget* /*w*/, void* /*v*/) {
		c::events::toggleInputRecording();
	});

	m_recTriggerMode.callback([](Fl_Widget* /*w*/, void* /*v*/) {
		c::main::toggleRecOnSignal();
	});

	m_inputRecMode.callback([](Fl_Widget* /*w*/, void* /*v*/) {
		c::main::toggleFreeInputRec();
	});

	m_metronome.type(FL_TOGGLE_BUTTON);
	m_metronome.callback([](Fl_Widget* /*w*/, void* /*v*/) {
		c::events::toggleMetronome();
	});
}

/* -------------------------------------------------------------------------- */

void geMainTransport::refresh()
{
	m_play.setStatus(m::clock::isRunning());
	m_recAction.setStatus(m::recManager::isRecordingAction());
	m_recInput.setStatus(m::recManager::isRecordingInput());
	m_metronome.setStatus(m::sequencer::isMetronomeOn());
	m_recTriggerMode.setStatus(m::conf::conf.recTriggerMode == RecTriggerMode::SIGNAL);
	m_inputRecMode.setStatus(m::conf::conf.inputRecMode == InputRecMode::FREE);
}
} // namespace giada::v
