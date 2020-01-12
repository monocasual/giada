/* -----------------------------------------------------------------------------
*
* Giada - Your Hardcore Loopmachine
*
* ------------------------------------------------------------------------------
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
* --------------------------------------------------------------------------- */


#include "core/const.h"
#include "core/model/model.h"
#include "core/graphics.h"
#include "core/mixer.h"
#include "core/mixerHandler.h"
#include "core/pluginHost.h"
#include "glue/main.h"
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
: Fl_Pack(x, y, 396, 20)
{
	type(Fl_Pack::HORIZONTAL);
	spacing(G_GUI_INNER_MARGIN);

	begin();

#if defined(WITH_VST)

	masterFxIn  = new geStatusButton(0, 0, G_GUI_UNIT, G_GUI_UNIT, fxOff_xpm, fxOn_xpm);
	inVol       = new geDial        (0, 0, G_GUI_UNIT, G_GUI_UNIT);
	inMeter     = new geSoundMeter  (0, 0, 140, G_GUI_UNIT);
	inToOut     = new geButton      (0, 0, 12, G_GUI_UNIT, "");
	outMeter    = new geSoundMeter  (0, 0, 140, G_GUI_UNIT);
	outVol      = new geDial        (0, 0, G_GUI_UNIT, G_GUI_UNIT);
	masterFxOut = new geStatusButton(0, 0, G_GUI_UNIT, G_GUI_UNIT, fxOff_xpm, fxOn_xpm);

#else

	inVol       = new geDial      (0, 0, G_GUI_UNIT, G_GUI_UNIT);
	inMeter     = new geSoundMeter(0, 0, 140, G_GUI_UNIT);
	outMeter    = new geSoundMeter(0, 0, 140, G_GUI_UNIT);
	outVol      = new geDial      (0, 0, G_GUI_UNIT, G_GUI_UNIT);

#endif

	end();

	resizable(nullptr);   // don't resize any widget

	outVol->callback(cb_outVol, (void*)this);
	inVol->callback(cb_inVol, (void*)this);

	outVol->value(m::mh::getOutVol());
	inVol->value(m::mh::getInVol());

#ifdef WITH_VST

	masterFxOut->callback(cb_masterFxOut, (void*)this);
	masterFxIn->callback(cb_masterFxIn, (void*)this);
	inToOut->callback(cb_inToOut, (void*)this);
	inToOut->type(FL_TOGGLE_BUTTON);

#endif
}


/* -------------------------------------------------------------------------- */


void geMainIO::cb_outVol     (Fl_Widget* v, void* p) { ((geMainIO*)p)->cb_outVol(); }
void geMainIO::cb_inVol      (Fl_Widget* v, void* p) { ((geMainIO*)p)->cb_inVol(); }
#ifdef WITH_VST
void geMainIO::cb_masterFxOut(Fl_Widget* v, void* p) { ((geMainIO*)p)->cb_masterFxOut(); }
void geMainIO::cb_masterFxIn (Fl_Widget* v, void* p) { ((geMainIO*)p)->cb_masterFxIn(); }
void geMainIO::cb_inToOut    (Fl_Widget* v, void* p) { ((geMainIO*)p)->cb_inToOut(); }
#endif


/* -------------------------------------------------------------------------- */


void geMainIO::cb_outVol()
{
	c::main::setOutVol(outVol->value());
}


/* -------------------------------------------------------------------------- */


void geMainIO::cb_inVol()
{
	c::main::setInVol(inVol->value());
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


void geMainIO::cb_inToOut()
{
	m::mh::setInToOut(inToOut->value());
}

#endif


/* -------------------------------------------------------------------------- */


void geMainIO::setOutVol(float v)
{
  outVol->value(v);
}


void geMainIO::setInVol(float v)
{
  inVol->value(v);
}


/* -------------------------------------------------------------------------- */


#ifdef WITH_VST

void geMainIO::setMasterFxOutFull(bool v)
{
  masterFxOut->setStatus(v);
}


void geMainIO::setMasterFxInFull(bool v)
{
  masterFxIn->setStatus(v);
}

#endif


/* -------------------------------------------------------------------------- */


void geMainIO::refresh()
{
	outMeter->mixerPeak = m::mixer::peakOut.load();
	inMeter->mixerPeak  = m::mixer::peakIn.load();
	outMeter->redraw();
	inMeter->redraw();
}


/* -------------------------------------------------------------------------- */


void geMainIO::rebuild()
{
	m::model::onGet(m::model::channels, m::mixer::MASTER_OUT_CHANNEL_ID, [&](m::Channel& c)
	{
		outVol->value(c.volume);
#ifdef WITH_VST
		masterFxOut->setStatus(c.pluginIds.size() > 0);
#endif
	});

	m::model::onGet(m::model::channels, m::mixer::MASTER_IN_CHANNEL_ID, [&](m::Channel& c)
	{
		inVol->value(c.volume);
#ifdef WITH_VST
		masterFxIn->setStatus(c.pluginIds.size() > 0);
#endif
	});
}
}} // giada::v::
