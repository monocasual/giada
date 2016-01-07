/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * glue
 * Intermediate layer GUI <-> CORE.
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


#include "../gui/dialogs/gd_mainWindow.h"
#include "../gui/elems/ge_keyboard.h"
#include "../gui/elems/ge_channel.h"
#include "../utils/gui_utils.h"
#include "../core/mixerHandler.h"
#include "../core/mixer.h"
#include "../core/conf.h"
#include "../core/channel.h"
#include "../core/sampleChannel.h"
#include "../core/midiChannel.h"
#include "glue.h"
#include "channel.h"


extern gdMainWindow *mainWin;
extern Conf          G_Conf;
extern Mixer	   		 G_Mixer;


using std::string;


int glue_loadChannel(SampleChannel *ch, const char *fname)
{
	/* save the patch and take the last browser's dir in order to re-use it
	 * the next time */

	G_Conf.samplePath = gDirname(fname);

	int result = ch->load(fname);

	if (result == SAMPLE_LOADED_OK)
		mainWin->keyboard->updateChannel(ch->guiChannel);

	return result;
}


/* -------------------------------------------------------------------------- */


Channel *glue_addChannel(int column, int type)
{
	Channel *ch    = G_Mixer.addChannel(type);
	gChannel *gch  = mainWin->keyboard->addChannel(column, ch);
	ch->guiChannel = gch;
	glue_setChanVol(ch, 1.0, false); // false = not from gui click
	return ch;
}


/* -------------------------------------------------------------------------- */


void glue_deleteChannel(Channel *ch)
{
	int index = ch->index;
	recorder::clearChan(index);
	Fl::lock();
	mainWin->keyboard->deleteChannel(ch->guiChannel);
	Fl::unlock();
	G_Mixer.deleteChannel(ch);
	gu_closeAllSubwindows();
}


/* -------------------------------------------------------------------------- */


void glue_freeChannel(Channel *ch)
{
	mainWin->keyboard->freeChannel(ch->guiChannel);
	recorder::clearChan(ch->index);
	ch->empty();
}


/* -------------------------------------------------------------------------- */


int glue_cloneChannel(Channel *src)
{
	Channel *ch    = G_Mixer.addChannel(src->type);
	gChannel *gch  = mainWin->keyboard->addChannel(src->guiChannel->getColumnIndex(), ch);

	ch->guiChannel = gch;
	ch->copy(src);

	mainWin->keyboard->updateChannel(ch->guiChannel);
	return true;
}
