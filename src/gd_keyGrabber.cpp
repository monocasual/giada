/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * gd_keyGrabber
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2013 Giovanni A. Zuliani | Monocasual
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
 * ------------------------------------------------------------------ */


#include "gd_keyGrabber.h"
#include "ge_mixed.h"
#include "conf.h"
#include "gui_utils.h"
#include "gd_config.h"
#include "channel.h"
#include "gg_keyboard.h"


extern Conf	G_Conf;


gdKeyGrabber::gdKeyGrabber(SampleChannel *ch)
: Fl_Window(300, 100, "Key configuration"), ch(ch) {
	set_modal();
	text = new gBox(10, 10, 280, 80, "Press a key (esc to quit):");
	gu_setFavicon(this);
	show();
}


/* ------------------------------------------------------------------ */


int gdKeyGrabber::handle(int e) {
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
				printf("set key '%c' (%d) for channel %d\n", x, x, ch->index);

				char tmp[2]; sprintf(tmp, "%c", x);
				ch->guiChannel->button->copy_label(tmp);
				ch->key = x;
				Fl::delete_widget(this);
				break;
			}
			else
				printf("invalid key\n");
		}
	}
	return(ret);
}
