/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ------------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2025 Giovanni A. Zuliani | Monocasual Laboratories
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

#include "gui/elems/mainWindow/mainOutput.h"
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
geMainOutput::geMainOutput()
: geFlex(Direction::VERTICAL, G_GUI_INNER_MARGIN)
{
	m_outMeter    = new geSoundMeter(Direction::VERTICAL);
	m_outVol      = new geDial(0, 0, 0, 0);
	m_masterFxOut = new geImageButton(graphics::fxOff, graphics::fxOn);
	m_midiOut     = new geMidiLed();

	addWidget(m_masterFxOut, G_GUI_UNIT);
	addWidget(m_outVol, G_GUI_UNIT);
	addWidget(m_outMeter);
	addWidget(m_midiOut, 10);
	end();

	m_masterFxOut->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_IO_LABEL_FXOUT));
	m_outVol->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_IO_LABEL_OUTVOL));
	m_outMeter->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_IO_LABEL_OUTMETER));
	m_midiOut->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_IO_LABEL_MIDIOUTACTIVITY));

	m_outVol->onChange = [](float v)
	{
		c::main::setMasterOutVolume(v, Thread::MAIN);
	};

	m_masterFxOut->forceValue(m_io.masterOutHasPlugins);
	m_masterFxOut->onClick = []
	{ c::layout::openMasterOutPluginListWindow(); };
}

/* -------------------------------------------------------------------------- */

void geMainOutput::setOutVol(float v) { m_outVol->value(v); }

/* -------------------------------------------------------------------------- */

void geMainOutput::refresh()
{
	m_outMeter->peak  = m_io.getMasterOutPeak();
	m_outMeter->ready = m_io.isKernelReady();
	m_outMeter->redraw();
	m_midiOut->redraw();
}

/* -------------------------------------------------------------------------- */

void geMainOutput::rebuild()
{
	m_io = c::main::getIO();

	m_outVol->value(m_io.masterOutVol);
	m_masterFxOut->setValue(m_io.masterOutHasPlugins);
}

/* -------------------------------------------------------------------------- */

void geMainOutput::setMidiOutActivity() { m_midiOut->lit(); }
} // namespace giada::v