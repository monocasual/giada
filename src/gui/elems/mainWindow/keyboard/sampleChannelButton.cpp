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


#include <FL/Fl.H>
#include "../../../../core/const.h"
#include "../../../../core/sampleChannel.h"
#include "../../../../utils/string.h"
#include "../../../../utils/fs.h"
#include "../../../../glue/channel.h"
#include "../../../dialogs/gd_mainWindow.h"
#include "sampleChannel.h"
#include "keyboard.h"
#include "sampleChannelButton.h"


extern gdMainWindow *G_MainWin;


geSampleChannelButton::geSampleChannelButton(int x, int y, int w, int h,
  const char *l)
	: geChannelButton(x, y, w, h, l)
{
}


/* -------------------------------------------------------------------------- */


int geSampleChannelButton::handle(int e)
{
	int ret = geButton::handle(e);
	switch (e) {
		case FL_DND_ENTER:
		case FL_DND_DRAG:
		case FL_DND_RELEASE: {
			ret = 1;
			break;
		}
		case FL_PASTE: {
      geSampleChannel *gch = static_cast<geSampleChannel*>(parent());
      SampleChannel   *ch  = static_cast<SampleChannel*>(gch->ch);
      int result = glue_loadChannel(ch, gu_trim(gu_stripFileUrl(Fl::event_text())).c_str());
			if (result != G_RES_OK)
				G_MainWin->keyboard->printChannelMessage(result);
			ret = 1;
			break;
		}
	}
	return ret;
}
