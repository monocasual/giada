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

#include "gui/elems/mainWindow/mainTransport.h"
#include "core/conf.h"
#include "core/const.h"
#include "core/graphics.h"
#include "glue/events.h"
#include "glue/main.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/flex.h"
#include "gui/elems/basics/statusButton.h"
#include "gui/ui.h"

extern giada::v::Ui g_ui;

namespace giada::v
{
geMainTransport::geMainTransport()
: geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN)
{
	m_rewind         = new geButton("", rewindOff_xpm, rewindOn_xpm);
	m_play           = new geStatusButton(play_xpm, pause_xpm);
	m_recTriggerMode = new geStatusButton(recTriggerModeOff_xpm, recTriggerModeOn_xpm);
	m_recAction      = new geStatusButton(recOff_xpm, recOn_xpm);
	m_recInput       = new geStatusButton(inputRecOff_xpm, inputRecOn_xpm);
	m_inputRecMode   = new geStatusButton(freeInputRecOff_xpm, freeInputRecOn_xpm);
	m_metronome      = new geStatusButton(metronomeOff_xpm, metronomeOn_xpm);
	add(m_rewind, 25);
	add(m_play, 25);
	add(new geBox(), 10);
	add(m_recTriggerMode, 15);
	add(m_recAction, 25);
	add(m_recInput, 25);
	add(m_inputRecMode, 15);
	add(new geBox(), 10);
	add(m_metronome, 15);
	end();

	m_rewind->copy_tooltip(g_ui.langMapper.get(LangMap::MAIN_TRANSPORT_LABEL_REWIND));
	m_play->copy_tooltip(g_ui.langMapper.get(LangMap::MAIN_TRANSPORT_LABEL_PLAY));
	m_recTriggerMode->copy_tooltip(g_ui.langMapper.get(LangMap::MAIN_TRANSPORT_LABEL_RECTRIGGERMODE));
	m_recAction->copy_tooltip(g_ui.langMapper.get(LangMap::MAIN_TRANSPORT_LABEL_RECACTIONS));
	m_recInput->copy_tooltip(g_ui.langMapper.get(LangMap::MAIN_TRANSPORT_LABEL_RECINPUT));
	m_inputRecMode->copy_tooltip(g_ui.langMapper.get(LangMap::MAIN_TRANSPORT_LABEL_RECINPUTMODE));
	m_metronome->copy_tooltip(g_ui.langMapper.get(LangMap::MAIN_TRANSPORT_LABEL_METRONOME));

	m_rewind->callback([](Fl_Widget* /*w*/, void* /*v*/) {
		c::events::rewindSequencer(Thread::MAIN);
	});

	m_play->callback([](Fl_Widget* /*w*/, void* /*v*/) {
		c::events::toggleSequencer(Thread::MAIN);
	});

	m_recAction->callback([](Fl_Widget* /*w*/, void* /*v*/) {
		c::events::toggleActionRecording();
	});

	m_recInput->callback([](Fl_Widget* /*w*/, void* /*v*/) {
		c::events::toggleInputRecording();
	});

	m_recTriggerMode->callback([](Fl_Widget* /*w*/, void* /*v*/) {
		c::main::toggleRecOnSignal();
	});

	m_inputRecMode->callback([](Fl_Widget* /*w*/, void* /*v*/) {
		c::main::toggleFreeInputRec();
	});

	m_metronome->type(FL_TOGGLE_BUTTON);
	m_metronome->callback([](Fl_Widget* /*w*/, void* /*v*/) {
		c::events::toggleMetronome();
	});
}

/* -------------------------------------------------------------------------- */

void geMainTransport::refresh()
{
	c::main::Transport transport = c::main::getTransport();

	m_play->setStatus(transport.isRunning);
	m_recAction->setStatus(transport.isRecordingAction);
	m_recInput->setStatus(transport.isRecordingInput);
	m_metronome->setStatus(transport.isMetronomeOn);
	m_recTriggerMode->setStatus(transport.recTriggerMode == RecTriggerMode::SIGNAL);
	m_inputRecMode->setStatus(transport.inputRecMode == InputRecMode::FREE);
}
} // namespace giada::v
