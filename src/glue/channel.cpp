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
#include "../gui/elems/channel.h"
#include "../utils/gui.h"
#include "../core/mixerHandler.h"
#include "../core/mixer.h"
#include "../core/pluginHost.h"
#include "../core/conf.h"
#include "../core/channel.h"
#include "../core/sampleChannel.h"
#include "../core/midiChannel.h"
#include "glue.h"
#include "channel.h"


extern gdMainWindow *G_MainWin;
extern Conf          G_Conf;
extern Recorder			 G_Recorder;
extern Mixer	   		 G_Mixer;
#ifdef WITH_VST
extern PluginHost G_PluginHost;
#endif


using std::string;


int glue_loadChannel(SampleChannel *ch, const char *fname)
{
	/* save the patch and take the last browser's dir in order to re-use it
	 * the next time */

	G_Conf.samplePath = gu_dirname(fname);

	int result = ch->load(fname, G_Conf.samplerate, G_Conf.rsmpQuality);

	if (result == SAMPLE_LOADED_OK)
		G_MainWin->keyboard->updateChannel(ch->guiChannel);

	return result;
}


/* -------------------------------------------------------------------------- */


Channel *glue_addChannel(int column, int type)
{
	Channel *ch     = G_Mixer.addChannel(type);
	geChannel *gch  = G_MainWin->keyboard->addChannel(column, ch);
	ch->guiChannel  = gch;
	glue_setChanVol(ch, 1.0, false); // false = not from gui click
	return ch;
}


/* -------------------------------------------------------------------------- */


void glue_deleteChannel(Channel *ch)
{
	G_Recorder.clearChan(ch->index);
#ifdef WITH_VST
	G_PluginHost.freeStack(PluginHost::CHANNEL, &G_Mixer.mutex_plugins, ch);
#endif
	Fl::lock();
	G_MainWin->keyboard->deleteChannel(ch->guiChannel);
	Fl::unlock();
	G_Mixer.deleteChannel(ch);
	gu_closeAllSubwindows();
}


/* -------------------------------------------------------------------------- */


void glue_freeChannel(Channel *ch)
{
#ifdef WITH_VST
	G_PluginHost.freeStack(PluginHost::CHANNEL, &G_Mixer.mutex_plugins, ch);
#endif
	G_MainWin->keyboard->freeChannel(ch->guiChannel);
	G_Recorder.clearChan(ch->index);
	ch->empty();
}


/* -------------------------------------------------------------------------- */


int glue_cloneChannel(Channel *src)
{
	Channel *ch    = G_Mixer.addChannel(src->type);
	geChannel *gch = G_MainWin->keyboard->addChannel(src->guiChannel->getColumnIndex(), ch);

	ch->guiChannel = gch;
	ch->copy(src, &G_Mixer.mutex_plugins);

	G_MainWin->keyboard->updateChannel(ch->guiChannel);
	return true;
}
