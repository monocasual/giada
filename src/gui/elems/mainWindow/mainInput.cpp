/* -----------------------------------------------------------------------------
*
* Giada - Your Hardcore Loopmachine
*
* ------------------------------------------------------------------------------
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
* --------------------------------------------------------------------------- */

#include "gui/elems/mainWindow/mainInput.h"
#include "core/const.h"
#include "glue/layout.h"
#include "glue/main.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/imageButton.h"
#include "gui/elems/basics/textButton.h"
#include "gui/elems/midiActivity.h"
#include "gui/elems/soundMeter.h"
#include "gui/graphics.h"
#include "gui/types.h"
#include "gui/ui.h"
#include "utils/gui.h"

extern giada::v::Ui* g_ui;

namespace giada::v
{
geMainInput::geMainInput()
: geFlex(Direction::VERTICAL, G_GUI_INNER_MARGIN)
{
	m_inMeter    = new geSoundMeter(Direction::VERTICAL);
	m_inVol      = new geDial(0, 0, 0, 0);
	m_inToOut    = new geImageButton(graphics::inToOutOff, graphics::inToOutOn);
	m_masterFxIn = new geImageButton(graphics::fxOff, graphics::fxOn);
	m_midiIn     = new geMidiLed();

	addWidget(m_masterFxIn, G_GUI_UNIT);
	addWidget(m_inVol, G_GUI_UNIT);
	addWidget(m_inMeter);
	addWidget(m_inToOut, G_GUI_UNIT);
	addWidget(m_midiIn, 10);
	end();

	m_inMeter->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_IO_LABEL_INMETER));
	m_inVol->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_IO_LABEL_INVOL));
	m_inToOut->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_IO_LABEL_INTOOUT));
	m_masterFxIn->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_IO_LABEL_FXIN));
	m_midiIn->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_IO_LABEL_MIDIINACTIVITY));

	m_inVol->onChange = [](float v) {
		c::main::setMasterInVolume(v, Thread::MAIN);
	};

	m_inToOut->setToggleable(true);
	m_inToOut->onClick = [&inToOut = m_inToOut]() {
		c::main::setInToOut(inToOut->getValue());
	};

	m_masterFxIn->onClick = [] { c::layout::openMasterInPluginListWindow(); };
}

/* -------------------------------------------------------------------------- */

void geMainInput::setInVol(float v) { m_inVol->value(v); }

/* -------------------------------------------------------------------------- */

void geMainInput::setMasterFxInFull(bool v)
{
	m_masterFxIn->forceValue(v);
}

/* -------------------------------------------------------------------------- */

void geMainInput::refresh()
{
	m_inMeter->peak  = m_io.getMasterInPeak();
	m_inMeter->ready = m_io.isKernelReady();
	m_inMeter->redraw();
	m_midiIn->redraw();
}

/* -------------------------------------------------------------------------- */

void geMainInput::rebuild()
{
	m_io = c::main::getIO();

	m_inVol->value(m_io.masterInVol);
	m_masterFxIn->setValue(m_io.masterInHasPlugins);
	m_inToOut->setValue(m_io.inToOut);
}

/* -------------------------------------------------------------------------- */

void geMainInput::setMidiInActivity() { m_midiIn->lit(); }
} // namespace giada::v