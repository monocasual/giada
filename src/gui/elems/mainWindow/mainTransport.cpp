/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2023 Giovanni A. Zuliani | Monocasual Laboratories
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
#include "core/const.h"
#include "glue/main.h"
#include "gui/elems/basics/box.h"
#include "gui/elems/basics/flex.h"
#include "gui/elems/basics/imageButton.h"
#include "gui/graphics.h"
#include "gui/ui.h"

extern giada::v::Ui* g_ui;

namespace giada::v
{
geMainTransport::geMainTransport()
: geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN)
{
	m_rewind         = new geImageButton(graphics::rewindOff, graphics::rewindOn);
	m_play           = new geImageButton(graphics::playOff, graphics::playOn);
	m_recTriggerMode = new geImageButton(graphics::recTriggerModeOff, graphics::recTriggerModeOn);
	m_recAction      = new geImageButton(graphics::actionRecOff, graphics::actionRecOn);
	m_recInput       = new geImageButton(graphics::inputRecOff, graphics::inputRecOn, graphics::inputRecDisabled);
	m_inputRecMode   = new geImageButton(graphics::freeInputRecOff, graphics::freeInputRecOn);
	m_metronome      = new geImageButton(graphics::metronomeOff, graphics::metronomeOn);
	addWidget(m_rewind, 25);
	addWidget(m_play, 25);
	addWidget(new geBox(), 10);
	addWidget(m_recTriggerMode, 15);
	addWidget(m_recAction, 25);
	addWidget(m_recInput, 25);
	addWidget(m_inputRecMode, 15);
	addWidget(new geBox(), 10);
	addWidget(m_metronome, 15);
	end();

	m_rewind->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_TRANSPORT_LABEL_REWIND));
	m_play->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_TRANSPORT_LABEL_PLAY));
	m_recTriggerMode->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_TRANSPORT_LABEL_RECTRIGGERMODE));
	m_recAction->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_TRANSPORT_LABEL_RECACTIONS));
	m_recInput->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_TRANSPORT_LABEL_RECINPUT));
	m_inputRecMode->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_TRANSPORT_LABEL_RECINPUTMODE));
	m_metronome->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_TRANSPORT_LABEL_METRONOME));

	m_rewind->onClick = []() {
		c::main::rewindSequencer();
	};

	m_play->setToggleable(true);
	m_play->onClick = []() {
		c::main::toggleSequencer();
	};

	m_recAction->setToggleable(true);
	m_recAction->onClick = []() {
		c::main::toggleActionRecording();
	};

	m_recInput->setToggleable(true);
	m_recInput->onClick = []() {
		c::main::toggleInputRecording();
	};

	m_recTriggerMode->setToggleable(true);
	m_recTriggerMode->onClick = []() {
		c::main::toggleRecOnSignal();
	};

	m_inputRecMode->setToggleable(true);
	m_inputRecMode->onClick = []() {
		c::main::toggleFreeInputRec();
	};

	m_metronome->setToggleable(true);
	m_metronome->onClick = []() {
		c::main::toggleMetronome();
	};
}

/* -------------------------------------------------------------------------- */

void geMainTransport::refresh()
{
	c::main::Transport transport = c::main::getTransport();

	m_play->setValue(transport.isRunning);
	m_recAction->setValue(transport.isRecordingAction);
	m_recInput->setValue(transport.isRecordingInput);
	m_metronome->setValue(transport.isMetronomeOn);
	m_recTriggerMode->setValue(transport.recTriggerMode == RecTriggerMode::SIGNAL);
	m_inputRecMode->setValue(transport.inputRecMode == InputRecMode::FREE);

	transport.canRecordInput() ? m_recInput->activate() : m_recInput->deactivate();
}
} // namespace giada::v
