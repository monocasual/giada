/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2019 Giovanni A. Zuliani | Monocasual
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


#include <FL/Fl.H>
#include "../../../../core/const.h"
#include "../../../../core/channel.h"
#include "../../../../core/graphics.h"
#include "../../../../core/pluginHost.h"
#include "../../../../utils/gui.h"
#include "../../../../glue/channel.h"
#include "../../../dialogs/mainWindow.h"
#include "../../../dialogs/pluginList.h"
#include "../../basics/button.h"
#include "../../basics/dial.h"
#include "../../basics/statusButton.h"
#include "column.h"
#include "channelStatus.h"
#include "channelButton.h"
#include "channel.h"


extern gdMainWindow* G_MainWin;


using namespace giada;


geChannel::geChannel(int X, int Y, int W, int H, giada::m::Channel* ch)
 : Fl_Group(X, Y, W, H, nullptr),
	 ch      (ch)
{
}


/* -------------------------------------------------------------------------- */


void geChannel::cb_arm(Fl_Widget* v, void* p) { ((geChannel*)p)->cb_arm(); }
void geChannel::cb_mute(Fl_Widget* v, void* p) { ((geChannel*)p)->cb_mute(); }
void geChannel::cb_solo(Fl_Widget* v, void* p) { ((geChannel*)p)->cb_solo(); }
void geChannel::cb_changeVol(Fl_Widget* v, void* p) { ((geChannel*)p)->cb_changeVol(); }
#ifdef WITH_VST
void geChannel::cb_openFxWindow(Fl_Widget* v, void* p) { ((geChannel*)p)->cb_openFxWindow(); }
#endif


/* -------------------------------------------------------------------------- */


void geChannel::cb_arm()
{
	c::channel::toggleArm(ch, true);
}


/* -------------------------------------------------------------------------- */


void geChannel::cb_mute()
{
	c::channel::toggleMute(ch);
}


/* -------------------------------------------------------------------------- */


void geChannel::cb_solo()
{
	c::channel::toggleSolo(ch);
}


/* -------------------------------------------------------------------------- */


void geChannel::cb_changeVol()
{
	c::channel::setVolume(ch, vol->value());
}


/* -------------------------------------------------------------------------- */


#ifdef WITH_VST
void geChannel::cb_openFxWindow()
{
	gu_openSubWindow(G_MainWin, new gdPluginList(m::pluginHost::StackType::CHANNEL, ch), WID_FX_LIST);
}
#endif


/* -------------------------------------------------------------------------- */



int geChannel::getColumnIndex()
{
	return static_cast<geColumn*>(parent())->getIndex();
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


void geChannel::setColorsByStatus(ChannelStatus playStatus, ChannelStatus recStatus)
{
	switch (playStatus) {
		case ChannelStatus::OFF:
		case ChannelStatus::EMPTY:
			mainButton->setDefaultMode();
			button->imgOn  = channelPlay_xpm;
			button->imgOff = channelStop_xpm;
			button->redraw();
			break;
		case ChannelStatus::PLAY:
			mainButton->setPlayMode();
			button->imgOn  = channelStop_xpm;
			button->imgOff = channelPlay_xpm;
			button->redraw();
			break;
		case ChannelStatus::WAIT:
			blink();
			break;
		case ChannelStatus::ENDING:
			mainButton->setEndingMode();
			break;
		default: break;
	}

	switch (recStatus) {
		case ChannelStatus::WAIT:
			blink();
			break;
		case ChannelStatus::ENDING:
			mainButton->setEndingMode();
			break;
		default: break;
	}
}


/* -------------------------------------------------------------------------- */


void geChannel::packWidgets()
{
	/* Count visible widgets and resize mainButton according to how many widgets
	are visible. */

	int visibles = 0;
	for (int i=0; i<children(); i++) {
		child(i)->size(MIN_ELEM_W, child(i)->h());  // also normalize widths
		if (child(i)->visible())
			visibles++;
	}
	mainButton->size(w() - ((visibles - 1) * (MIN_ELEM_W + G_GUI_INNER_MARGIN)),   // -1: exclude itself
		mainButton->h());

	/* Reposition everything else */

	for (int i=1, p=0; i<children(); i++) {
		if (!child(i)->visible())
			continue;
		for (int k=i-1; k>=0; k--) // Get the first visible item prior to i
			if (child(k)->visible()) {
				p = k;
				break;
			}
		child(i)->position(child(p)->x() + child(p)->w() + G_GUI_INNER_MARGIN, child(i)->y());
	}

	init_sizes(); // Resets the internal array of widget sizes and positions
}


/* -------------------------------------------------------------------------- */


int geChannel::handleKey(int e, int key)
{
	int ret;
	if (e == FL_KEYDOWN && button->value())                                // key already pressed! skip it
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


/* -------------------------------------------------------------------------- */


void geChannel::changeSize(int H)
{
	size(w(), H);
	
	int Y = y() + (H / 2 - (G_GUI_UNIT / 2));

	button->resize(x(), Y, w(), G_GUI_UNIT);
	arm->resize(x(), Y, w(), G_GUI_UNIT);   
	mainButton->resize(x(), y(), w(), H);
	mute->resize(x(), Y, w(), G_GUI_UNIT);
	solo->resize(x(), Y, w(), G_GUI_UNIT);
	vol->resize(x(), Y, w(), G_GUI_UNIT);
#ifdef WITH_VST
	fx->resize(x(), Y, w(), G_GUI_UNIT);
#endif
}


/* -------------------------------------------------------------------------- */


int geChannel::getSize()
{
	return h();
}
