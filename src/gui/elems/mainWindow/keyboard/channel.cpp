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


#include "../../../../core/const.h"
#include "../../../../core/channel.h"
#include "../../../../core/graphics.h"
#include "../../../../core/pluginHost.h"
#include "../../../../utils/gui.h"
#include "../../../../glue/channel.h"
#include "../../../dialogs/gd_mainWindow.h"
#include "../../../dialogs/gd_pluginList.h"
#include "../../basics/idButton.h"
#include "../../basics/dial.h"
#include "column.h"
#include "channelButton.h"
#include "channel.h"


extern gdMainWindow *G_MainWin;


using namespace giada;


geChannel::geChannel(int X, int Y, int W, int H, int type, Channel *ch)
 : Fl_Group(X, Y, W, H, nullptr),
   ch      (ch),
   type    (type)
{
}


/* -------------------------------------------------------------------------- */


void geChannel::cb_arm(Fl_Widget *v, void *p) { ((geChannel*)p)->__cb_arm(); }
void geChannel::cb_mute(Fl_Widget *v, void *p) { ((geChannel*)p)->__cb_mute(); }
void geChannel::cb_solo(Fl_Widget *v, void *p) { ((geChannel*)p)->__cb_solo(); }
void geChannel::cb_changeVol(Fl_Widget *v, void *p) { ((geChannel*)p)->__cb_changeVol(); }
#ifdef WITH_VST
void geChannel::cb_openFxWindow(Fl_Widget *v, void *p) { ((geChannel*)p)->__cb_openFxWindow(); }
#endif


/* -------------------------------------------------------------------------- */


void geChannel::__cb_arm()
{
  glue_toggleArm(ch, true);
}


/* -------------------------------------------------------------------------- */


void geChannel::__cb_mute()
{
	glue_setMute(ch);
}


/* -------------------------------------------------------------------------- */


void geChannel::__cb_solo()
{
	solo->value() ? glue_setSoloOn(ch) : glue_setSoloOff(ch);
}


/* -------------------------------------------------------------------------- */


void geChannel::__cb_changeVol()
{
	glue_setChanVol(ch, vol->value());
}


/* -------------------------------------------------------------------------- */


#ifdef WITH_VST
void geChannel::__cb_openFxWindow()
{
	gu_openSubWindow(G_MainWin, new gdPluginList(pluginHost::CHANNEL, ch), WID_FX_LIST);
}
#endif


/* -------------------------------------------------------------------------- */


int geChannel::keyPress(int e)
{
	return handleKey(e, ch->key);
}


/* -------------------------------------------------------------------------- */



int geChannel::getColumnIndex()
{
	return ((geColumn*)parent())->getIndex();
}


/* -------------------------------------------------------------------------- */


void geChannel::blink()
{
	if (gu_getBlinker() > 6)
		mainButton->setPlayMode();
	else
    mainButton->setDefaultMode();
}


/* -------------------------------------------------------------------------- */


void geChannel::setColorsByStatus(int playStatus, int recStatus)
{
  switch (playStatus) {
    case STATUS_OFF:
    case STATUS_EMPTY:
  		mainButton->setDefaultMode();
      button->imgOn  = channelPlay_xpm;
      button->imgOff = channelStop_xpm;
      button->redraw();
      break;
    case STATUS_PLAY:
      mainButton->setPlayMode();
      button->imgOn  = channelStop_xpm;
      button->imgOff = channelPlay_xpm;
      button->redraw();
      break;
    case STATUS_WAIT:
      blink();
      break;
    case STATUS_ENDING:
      mainButton->setEndingMode();
      break;
  }

  switch (recStatus) {
    case REC_WAITING:
      blink();
      break;
    case REC_ENDING:
      mainButton->setEndingMode();
      break;
  }
}


/* -------------------------------------------------------------------------- */


void geChannel::packWidgets()
{
  /* Count visible widgets and resize mainButton according to how many widgets
  are visible. */

  int visibles = 0;
  for (int i=0; i<children(); i++) {
    child(i)->size(20, 20);  // also normalize widths
    if (child(i)->visible())
      visibles++;
  }
  mainButton->size(w() - ((visibles - 1) * (24)), 20);  // -1: exclude itself

  /* Reposition everything else */

  for (int i=1, p=0; i<children(); i++) {
    if (!child(i)->visible())
      continue;
    for (int k=i-1; k>=0; k--) // Get the first visible item prior to i
      if (child(k)->visible()) {
        p = k;
        break;
      }
    child(i)->position(child(p)->x() + child(p)->w() + 4, y());
  }

  init_sizes(); // Resets the internal array of widget sizes and positions
}


/* -------------------------------------------------------------------------- */


int geChannel::handleKey(int e, int key)
{
	int ret;
	if (e == FL_KEYDOWN && button->value())                              // key already pressed! skip it
		ret = 1;
	else
	if (Fl::event_key() == key && !button->value()) {
		button->take_focus();                                              // move focus to this button
		button->value((e == FL_KEYDOWN || e == FL_SHORTCUT) ? 1 : 0);      // change the button's state
		button->do_callback();                                             // invoke the button's callback
		ret = 1;
	}
	else
		ret = 0;

	if (Fl::event_key() == key)
		button->value((e == FL_KEYDOWN || e == FL_SHORTCUT) ? 1 : 0);      // change the button's state

	return ret;
}
