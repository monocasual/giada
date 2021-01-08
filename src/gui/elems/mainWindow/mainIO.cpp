/* -----------------------------------------------------------------------------
*
* Giada - Your Hardcore Loopmachine
*
* ------------------------------------------------------------------------------
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
* --------------------------------------------------------------------------- */


#include "core/const.h"
#include "core/graphics.h"
#include "glue/events.h"
#include "glue/main.h"
#include "glue/channel.h"
#include "utils/gui.h"
#include "gui/elems/soundMeter.h"
#include "gui/elems/basics/statusButton.h"
#include "gui/elems/basics/dial.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/pluginList.h"
#include "mainIO.h"


extern giada::v::gdMainWindow* G_MainWin;


namespace giada {
namespace v
{
geMainIO::geMainIO(int x, int y)
: gePack     (x, y, Direction::HORIZONTAL)
, outMeter   (0, 0, 140, G_GUI_UNIT)
, inMeter    (0, 0, 140, G_GUI_UNIT)
, outVol     (0, 0, G_GUI_UNIT, G_GUI_UNIT)
, inVol      (0, 0, G_GUI_UNIT, G_GUI_UNIT)
, inToOut    (0, 0, 12, G_GUI_UNIT, "")
#ifdef WITH_VST
, masterFxOut(0, 0, G_GUI_UNIT, G_GUI_UNIT, fxOff_xpm, fxOn_xpm)
, masterFxIn (0, 0, G_GUI_UNIT, G_GUI_UNIT, fxOff_xpm, fxOn_xpm)
#endif
{
#ifdef WITH_VST
	add(&masterFxIn);
#endif
	add(&inVol);
	add(&inMeter);
	add(&inToOut);
	add(&outMeter);
	add(&outVol);
#ifdef WITH_VST
	add(&masterFxOut);
#endif

	resizable(nullptr);   // don't resize any widget

	outVol.callback(cb_outVol, (void*)this);
	inVol.callback(cb_inVol, (void*)this);

	inToOut.callback(cb_inToOut, (void*)this);
	inToOut.type(FL_TOGGLE_BUTTON);

#ifdef WITH_VST
	masterFxOut.callback(cb_masterFxOut, (void*)this);
	masterFxIn.callback(cb_masterFxIn, (void*)this);
#endif
}


/* -------------------------------------------------------------------------- */


void geMainIO::cb_outVol     (Fl_Widget* /*w*/, void* p) { ((geMainIO*)p)->cb_outVol(); }
void geMainIO::cb_inVol      (Fl_Widget* /*w*/, void* p) { ((geMainIO*)p)->cb_inVol(); }
void geMainIO::cb_inToOut    (Fl_Widget* /*w*/, void* p) { ((geMainIO*)p)->cb_inToOut(); }
#ifdef WITH_VST
void geMainIO::cb_masterFxOut(Fl_Widget* /*w*/, void* p) { ((geMainIO*)p)->cb_masterFxOut(); }
void geMainIO::cb_masterFxIn (Fl_Widget* /*w*/, void* p) { ((geMainIO*)p)->cb_masterFxIn(); }
#endif


/* -------------------------------------------------------------------------- */


void geMainIO::cb_outVol()
{
	c::events::setMasterOutVolume(outVol.value(), Thread::MAIN);
}


void geMainIO::cb_inVol()
{
	c::events::setMasterInVolume(inVol.value(), Thread::MAIN);
}


void geMainIO::cb_inToOut()
{
	c::main::setInToOut(inToOut.value());
}

/* -------------------------------------------------------------------------- */


#ifdef WITH_VST

void geMainIO::cb_masterFxOut()
{
	u::gui::openSubWindow(G_MainWin, new v::gdPluginList(m::mixer::MASTER_OUT_CHANNEL_ID), WID_FX_LIST);
}


void geMainIO::cb_masterFxIn()
{
	u::gui::openSubWindow(G_MainWin, new v::gdPluginList(m::mixer::MASTER_IN_CHANNEL_ID), WID_FX_LIST);
}

#endif


/* -------------------------------------------------------------------------- */


void geMainIO::setOutVol(float v)
{
	outVol.value(v);
}


void geMainIO::setInVol(float v)
{
	inVol.value(v);
}


/* -------------------------------------------------------------------------- */


#ifdef WITH_VST

void geMainIO::setMasterFxOutFull(bool v)
{
	masterFxOut.setStatus(v);
}


void geMainIO::setMasterFxInFull(bool v)
{
	masterFxIn.setStatus(v);
}

#endif


/* -------------------------------------------------------------------------- */


void geMainIO::refresh()
{
	outMeter.mixerPeak = m_io.getMasterOutPeak();
	inMeter.mixerPeak  = m_io.getMasterInPeak();
	outMeter.redraw();
	inMeter.redraw();
}


/* -------------------------------------------------------------------------- */


void geMainIO::rebuild()
{
	m_io = c::main::getIO();

	outVol.value(m_io.masterOutVol);
	inVol.value(m_io.masterInVol);
#ifdef WITH_VST
	masterFxOut.setStatus(m_io.masterOutHasPlugins);
	masterFxIn.setStatus(m_io.masterInHasPlugins);
	inToOut.value(m_io.inToOut);
#endif
}
}} // giada::v::
