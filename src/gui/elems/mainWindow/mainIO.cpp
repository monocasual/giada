/* -----------------------------------------------------------------------------
*
* Giada - Your Hardcore Loopmachine
*
* ------------------------------------------------------------------------------
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
* --------------------------------------------------------------------------- */

#include "gui/elems/mainWindow/mainIO.h"
#include "core/const.h"
#include "core/graphics.h"
#include "glue/channel.h"
#include "glue/events.h"
#include "glue/layout.h"
#include "glue/main.h"
#include "gui/elems/basics/button.h"
#include "gui/elems/basics/dial.h"
#include "gui/elems/basics/statusButton.h"
#include "gui/elems/soundMeter.h"
#include "utils/gui.h"
#ifdef WITH_VST
#include "gui/elems/basics/statusButton.h"
#endif

namespace giada::v
{
geMainIO::geMainIO(int x, int y, int w, int h)
: geFlex(x, y, w, h, Direction::HORIZONTAL, G_GUI_INNER_MARGIN)
{
	m_outMeter = new geSoundMeter(0, 0, 0, 0);
	m_inMeter  = new geSoundMeter(0, 0, 0, 0);
	m_outVol   = new geDial(0, 0, 0, 0);
	m_inVol    = new geDial(0, 0, 0, 0);
	m_inToOut  = new geButton();
#ifdef WITH_VST
	m_masterFxOut = new geStatusButton(0, 0, 0, 0, fxOff_xpm, fxOn_xpm);
	m_masterFxIn  = new geStatusButton(0, 0, 0, 0, fxOff_xpm, fxOn_xpm);
#endif

#ifdef WITH_VST
	add(m_masterFxIn, G_GUI_UNIT);
#endif
	add(m_inVol, G_GUI_UNIT);
	add(m_inMeter);
	add(m_inToOut, 12);
	add(m_outMeter);
	add(m_outVol, G_GUI_UNIT);
#ifdef WITH_VST
	add(m_masterFxOut, G_GUI_UNIT);
#endif
	end();

	m_outMeter->copy_tooltip("Main output meter");
	m_inMeter->copy_tooltip("Main input meter");
	m_outVol->copy_tooltip("Main output volume");
	m_inVol->copy_tooltip("Main input volume");
	m_inToOut->copy_tooltip("Stream linker\n\nConnects input to output to enable \"hear what you're playing\" mode.");
#ifdef WITH_VST
	m_masterFxOut->copy_tooltip("Main output plug-ins");
	m_masterFxIn->copy_tooltip("Main input plug-ins");
#endif

	m_outVol->onChange = [](float v) {
		c::events::setMasterOutVolume(v, Thread::MAIN);
	};

	m_inVol->onChange = [](float v) {
		c::events::setMasterInVolume(v, Thread::MAIN);
	};

	m_inToOut->type(FL_TOGGLE_BUTTON);
	m_inToOut->onClick = [v = m_inToOut->value()]() {
		c::main::setInToOut(v);
	};

#ifdef WITH_VST
	m_masterFxOut->onClick = [] { c::layout::openMasterOutPluginListWindow(); };
	m_masterFxIn->onClick  = [] { c::layout::openMasterInPluginListWindow(); };
#endif
}

/* -------------------------------------------------------------------------- */

void geMainIO::setOutVol(float v) { m_outVol->value(v); }
void geMainIO::setInVol(float v) { m_inVol->value(v); }

/* -------------------------------------------------------------------------- */

#ifdef WITH_VST

void geMainIO::setMasterFxOutFull(bool v)
{
	m_masterFxOut->setStatus(v);
}

void geMainIO::setMasterFxInFull(bool v)
{
	m_masterFxIn->setStatus(v);
}

#endif

/* -------------------------------------------------------------------------- */

void geMainIO::refresh()
{
	m_outMeter->peak  = m_io.getMasterOutPeak();
	m_outMeter->ready = m_io.isKernelReady();
	m_inMeter->peak   = m_io.getMasterInPeak();
	m_inMeter->ready  = m_io.isKernelReady();
	m_outMeter->redraw();
	m_inMeter->redraw();
}

/* -------------------------------------------------------------------------- */

void geMainIO::rebuild()
{
	m_io = c::main::getIO();

	m_outVol->value(m_io.masterOutVol);
	m_inVol->value(m_io.masterInVol);
#ifdef WITH_VST
	m_masterFxOut->setStatus(m_io.masterOutHasPlugins);
	m_masterFxIn->setStatus(m_io.masterInHasPlugins);
	m_inToOut->value(m_io.inToOut);
#endif
}
} // namespace giada::v