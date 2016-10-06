/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_keyGrabber
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


#include "../../utils/gui.h"
#include "../../core/conf.h"
#include "../../core/channel.h"
#include "../../core/sampleChannel.h"
#include "../../core/midiChannel.h"
#include "../../utils/log.h"
#include "../elems/ge_keyboard.h"
#include "../elems/ge_mixed.h"
#include "../elems/channel.h"
#include "../elems/channelButton.h"
#include "gd_keyGrabber.h"
#include "gd_config.h"
#include "gd_mainWindow.h"


extern Conf	         G_Conf;
extern gdMainWindow *mainWin;


gdKeyGrabber::gdKeyGrabber(Channel *ch)
	: gWindow(300, 126, "Key configuration"), ch(ch)
{
	set_modal();
	text   = new gBox(8, 8, 284, 80, "");
	clear  = new gClick(w()-88, text->y()+text->h()+8, 80, 20, "Clear");
	cancel = new gClick(clear->x()-88, clear->y(), 80, 20, "Close");
	end();

	clear->callback(cb_clear, (void*)this);
	cancel->callback(cb_cancel, (void*)this);

	updateText(ch->key);

	gu_setFavicon(this);
	show();
}


/* -------------------------------------------------------------------------- */


void gdKeyGrabber::cb_clear (Fl_Widget *w, void *p) { ((gdKeyGrabber*)p)->__cb_clear(); }
void gdKeyGrabber::cb_cancel(Fl_Widget *w, void *p) { ((gdKeyGrabber*)p)->__cb_cancel(); }


/* -------------------------------------------------------------------------- */


void gdKeyGrabber::__cb_cancel()
{
	do_callback();
}


/* -------------------------------------------------------------------------- */


void gdKeyGrabber::__cb_clear()
{
	updateText(0);
	setButtonLabel(0);
}


/* -------------------------------------------------------------------------- */


void gdKeyGrabber::setButtonLabel(int key)
{
	char tmp[2]; sprintf(tmp, "%c", key);
	ch->guiChannel->mainButton->setKey(tmp);
	ch->key = key;
}

/* -------------------------------------------------------------------------- */


void gdKeyGrabber::updateText(int key)
{
	char tmp2[64];
	if (key != 0)
		sprintf(tmp2, "Press a key.\n\nCurrent binding: %c", key);
	else
		sprintf(tmp2, "Press a key.\n\nCurrent binding: [none]");
	text->copy_label(tmp2);
}


/* -------------------------------------------------------------------------- */


int gdKeyGrabber::handle(int e)
{
	int ret = Fl_Group::handle(e);
	switch(e) {
		case FL_KEYUP: {
			int x = Fl::event_key();
			if (strlen(Fl::event_text()) != 0
			    && x != FL_BackSpace
			    && x != FL_Enter
			    && x != FL_Delete
			    && x != FL_Tab
			    && x != FL_End
			    && x != ' ')
			{
				gu_log("set key '%c' (%d) for channel %d\n", x, x, ch->index);
				setButtonLabel(x);
				updateText(x);
				break;
			}
			else
				gu_log("invalid key\n");
		}
	}
	return(ret);
}
