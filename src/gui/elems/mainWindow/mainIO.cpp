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

#include "gui/elems/mainWindow/mainIO.h"
#include "core/const.h"
#include "glue/channel.h"
#include "glue/layout.h"
#include "glue/main.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/imageButton.h"
#include "gui/elems/basics/textButton.h"
#include "gui/elems/midiActivity.h"
#include "gui/elems/soundMeter.h"
#include "gui/graphics.h"
#include "gui/ui.h"
#include "utils/gui.h"

extern giada::v::Ui* g_ui;

namespace giada::v
{
geMainIO::geMainIO()
: geFlex(Direction::HORIZONTAL, G_GUI_INNER_MARGIN)
{
	m_outMeter     = new geSoundMeter(0, 0, 0, 0);
	m_inMeter      = new geSoundMeter(0, 0, 0, 0);
	m_outVol       = new geDial(0, 0, 0, 0);
	m_inVol        = new geDial(0, 0, 0, 0);
	m_inToOut      = new geTextButton("");
	m_masterFxOut  = new geImageButton(graphics::fxOff, graphics::fxOn);
	m_masterFxIn   = new geImageButton(graphics::fxOff, graphics::fxOn);
	m_midiActivity = new geMidiActivity();

	addWidget(m_masterFxIn, G_GUI_UNIT);
	addWidget(m_inVol, G_GUI_UNIT);
	addWidget(m_inMeter);
	addWidget(m_inToOut, 12);
	addWidget(m_outMeter);
	addWidget(m_outVol, G_GUI_UNIT);
	addWidget(m_masterFxOut, G_GUI_UNIT);
	addWidget(m_midiActivity, 10);
	end();

	m_outMeter->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_IO_LABEL_OUTMETER));
	m_inMeter->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_IO_LABEL_INMETER));
	m_outVol->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_IO_LABEL_OUTVOL));
	m_inVol->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_IO_LABEL_INVOL));
	m_inToOut->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_IO_LABEL_INTOOUT));
	m_masterFxOut->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_IO_LABEL_FXOUT));
	m_masterFxIn->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_IO_LABEL_FXIN));
	m_midiActivity->copy_tooltip(g_ui->getI18Text(LangMap::MAIN_IO_LABEL_MIDIACTIVITY));

	m_outVol->onChange = [](float v) {
		c::main::setMasterOutVolume(v, Thread::MAIN);
	};

	m_inVol->onChange = [](float v) {
		c::main::setMasterInVolume(v, Thread::MAIN);
	};

	m_inToOut->setToggleable(true);
	m_inToOut->onClick = [&inToOut = m_inToOut]() {
		c::main::setInToOut(inToOut->getValue());
	};

	m_masterFxOut->onClick = [] { c::layout::openMasterOutPluginListWindow(); };

	m_masterFxIn->onClick = [] { c::layout::openMasterInPluginListWindow(); };
}

/* -------------------------------------------------------------------------- */

void geMainIO::setOutVol(float v) { m_outVol->value(v); }
void geMainIO::setInVol(float v) { m_inVol->value(v); }

/* -------------------------------------------------------------------------- */

void geMainIO::setMasterFxOutFull(bool v)
{
	m_masterFxOut->forceValue(v);
}

void geMainIO::setMasterFxInFull(bool v)
{
	m_masterFxIn->forceValue(v);
}

/* -------------------------------------------------------------------------- */

void geMainIO::refresh()
{
	m_outMeter->peak  = m_io.getMasterOutPeak();
	m_outMeter->ready = m_io.isKernelReady();
	m_inMeter->peak   = m_io.getMasterInPeak();
	m_inMeter->ready  = m_io.isKernelReady();
	m_outMeter->redraw();
	m_inMeter->redraw();
	m_midiActivity->redraw();
}

/* -------------------------------------------------------------------------- */

void geMainIO::rebuild()
{
	m_io = c::main::getIO();

	m_outVol->value(m_io.masterOutVol);
	m_inVol->value(m_io.masterInVol);
	m_masterFxOut->setValue(m_io.masterOutHasPlugins);
	m_masterFxIn->setValue(m_io.masterInHasPlugins);
	m_inToOut->setValue(m_io.inToOut);
}

/* -------------------------------------------------------------------------- */

void geMainIO::setMidiOutActivity() { m_midiActivity->out->lit(); }
void geMainIO::setMidiInActivity() { m_midiActivity->in->lit(); }
} // namespace giada::v