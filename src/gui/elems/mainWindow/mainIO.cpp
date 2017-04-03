/* -----------------------------------------------------------------------------
*
* Giada - Your Hardcore Loopmachine
*
* -----------------------------------------------------------------------------
*
* Copyright (C) 2010-2017 Giovanni A. Zuliani | Monocasual
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


#include "../../../core/const.h"
#include "../../../core/graphics.h"
#include "../../../core/mixer.h"
#include "../../../core/pluginHost.h"
#include "../../../glue/main.h"
#include "../../../utils/gui.h"
#include "../../elems/ge_mixed.h"
#include "../../dialogs/gd_mainWindow.h"
#include "../../dialogs/gd_pluginList.h"
#include "mainIO.h"


extern gdMainWindow *G_MainWin;


using namespace giada;


geMainIO::geMainIO(int x, int y)
	: Fl_Group(x, y, 396, 20)
{
	begin();

#if defined(WITH_VST)
	masterFxIn  = new gFxButton  (x, y, 20, 20, fxOff_xpm, fxOn_xpm);
	inVol		    = new gDial      (masterFxIn->x()+masterFxIn->w()+4, y, 20, 20);
	inMeter     = new gSoundMeter(inVol->x()+inVol->w()+4, y+4, 140, 12);
	inToOut     = new gClick     (inMeter->x()+inMeter->w()+4, y+4, 12, 12, "", inputToOutputOff_xpm, inputToOutputOn_xpm);
	outMeter    = new gSoundMeter(inToOut->x()+inToOut->w()+4, y+4, 140, 12);
	outVol		  = new gDial      (outMeter->x()+outMeter->w()+4, y, 20, 20);
	masterFxOut = new gFxButton  (outVol->x()+outVol->w()+4, y, 20, 20, fxOff_xpm, fxOn_xpm);
#else
	inVol		    = new gDial      (x+62, y, 20, 20);
	inMeter     = new gSoundMeter(inVol->x()+inVol->w()+4, y+5, 140, 12);
	outMeter    = new gSoundMeter(inMeter->x()+inMeter->w()+4, y+5, 140, 12);
	outVol		  = new gDial      (outMeter->x()+outMeter->w()+4, y, 20, 20);
#endif

	end();

	resizable(nullptr);   // don't resize any widget

	outVol->callback(cb_outVol, (void*)this);
	outVol->value(mixer::outVol);
	inVol->callback(cb_inVol, (void*)this);
	inVol->value(mixer::inVol);

#ifdef WITH_VST
	masterFxOut->callback(cb_masterFxOut, (void*)this);
	masterFxIn->callback(cb_masterFxIn, (void*)this);
	inToOut->callback(cb_inToOut, (void*)this);
	inToOut->type(FL_TOGGLE_BUTTON);
#endif
}


/* -------------------------------------------------------------------------- */


void geMainIO::cb_outVol     (Fl_Widget *v, void *p)  	{ ((geMainIO*)p)->__cb_outVol(); }
void geMainIO::cb_inVol      (Fl_Widget *v, void *p)  	{ ((geMainIO*)p)->__cb_inVol(); }
#ifdef WITH_VST
void geMainIO::cb_masterFxOut(Fl_Widget *v, void *p)    { ((geMainIO*)p)->__cb_masterFxOut(); }
void geMainIO::cb_masterFxIn (Fl_Widget *v, void *p)    { ((geMainIO*)p)->__cb_masterFxIn(); }
void geMainIO::cb_inToOut    (Fl_Widget *v, void *p)    { ((geMainIO*)p)->__cb_inToOut(); }
#endif


/* -------------------------------------------------------------------------- */


void geMainIO::__cb_outVol()
{
	glue_setOutVol(outVol->value());
}


/* -------------------------------------------------------------------------- */


void geMainIO::__cb_inVol()
{
	glue_setInVol(inVol->value());
}


/* -------------------------------------------------------------------------- */


#ifdef WITH_VST
void geMainIO::__cb_masterFxOut()
{
	gu_openSubWindow(G_MainWin, new gdPluginList(pluginHost::MASTER_OUT), WID_FX_LIST);
}

void geMainIO::__cb_masterFxIn()
{
	gu_openSubWindow(G_MainWin, new gdPluginList(pluginHost::MASTER_IN), WID_FX_LIST);
}

void geMainIO::__cb_inToOut()
{
	mixer::inToOut = inToOut->value();
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
  masterFxOut->full = v;
  masterFxOut->redraw();
}


void geMainIO::setMasterFxInFull(bool v)
{
  masterFxIn->full = v;
  masterFxIn->redraw();
}

#endif


/* -------------------------------------------------------------------------- */


void geMainIO::refresh()
{
	outMeter->mixerPeak = mixer::peakOut;
	inMeter->mixerPeak  = mixer::peakIn;
	outMeter->redraw();
	inMeter->redraw();
}
