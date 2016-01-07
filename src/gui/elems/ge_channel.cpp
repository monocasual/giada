/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * ge_channel
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2016 Giovanni A. Zuliani | Monocasual
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


#include "../../core/pluginHost.h"
#include "../../core/mixer.h"
#include "../../core/conf.h"
#include "../../core/patch_DEPR_.h"
#include "../../core/graphics.h"
#include "../../core/channel.h"
#include "../../core/wave.h"
#include "../../core/sampleChannel.h"
#include "../../core/midiChannel.h"
#include "../../glue/glue.h"
#include "../../utils/gui_utils.h"
#include "../dialogs/gd_mainWindow.h"
#include "../dialogs/gd_keyGrabber.h"
#include "../dialogs/gd_midiInput.h"
#include "../dialogs/gd_editor.h"
#include "../dialogs/gd_actionEditor.h"
#include "../dialogs/gd_warnings.h"
#include "../dialogs/gd_browser.h"
#include "../dialogs/gd_midiOutput.h"
#include "ge_keyboard.h"
#include "ge_channel.h"
#include "ge_sampleChannel.h"

#ifdef WITH_VST
#include "../dialogs/gd_pluginList.h"
#endif


extern Mixer 		     G_Mixer;
extern Conf  		     G_Conf;
extern Patch_DEPR_   G_Patch_DEPR_;
extern gdMainWindow *mainWin;


gChannel::gChannel(int X, int Y, int W, int H, int type)
 : Fl_Group(X, Y, W, H, NULL), type(type) {}


/* -------------------------------------------------------------------------- */


int gChannel::getColumnIndex()
{
	return ((gColumn*)parent())->getIndex();
}


/* -------------------------------------------------------------------------- */


void gChannel::blink()
{
	if (gu_getBlinker() > 6)
		mainButton->setPlayMode();
	else
    mainButton->setDefaultMode();
}


/* -------------------------------------------------------------------------- */


void gChannel::setColorsByStatus(int playStatus, int recStatus)
{
  switch (playStatus) {
    case STATUS_OFF:
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


int gChannel::handleKey(int e, int key)
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
