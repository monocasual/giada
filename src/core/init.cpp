/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * init
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2015 Giovanni A. Zuliani | Monocasual
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


#include <ctime>
#include "../utils/log.h"
#include "../utils/utils.h"
#include "../utils/gui_utils.h"
#include "../gui/dialogs/gd_mainWindow.h"
#include "../gui/dialogs/gd_warnings.h"
#include "init.h"
#include "mixer.h"
#include "wave.h"
#include "const.h"
#include "mixerHandler.h"
#include "patch_DEPR_.h"
#include "patch.h"
#include "conf.h"
#include "pluginHost.h"
#include "recorder.h"
#include "midiMapConf.h"
#include "kernelMidi.h"


extern Mixer 			   G_Mixer;
extern bool		 		   G_audio_status;
extern bool		 		   G_quit;
extern Patch_DEPR_   G_Patch_DEPR_;
extern Patch         G_Patch;
extern Conf          G_Conf;
extern MidiMapConf   G_MidiMap;
extern gdMainWindow *mainWin;

#ifdef WITH_VST
extern PluginHost	   G_PluginHost;
#endif


void init_prepareParser()
{
	time_t t;
  time (&t);
	gLog("[init] Giada " G_VERSION_STR " - %s", ctime(&t));

	G_Conf.read();
	G_Patch_DEPR_.setDefault();
	G_Patch.init();

	if (!gLog_init(G_Conf.logMode))
		gLog("[init] log init failed! Using default stdout\n");

	gLog("[init] configuration file ready\n");
}


/* -------------------------------------------------------------------------- */


void init_prepareKernelAudio()
{
	kernelAudio::openDevice(
		G_Conf.soundSystem,
		G_Conf.soundDeviceOut,
		G_Conf.soundDeviceIn,
		G_Conf.channelsOut,
		G_Conf.channelsIn,
		G_Conf.samplerate,
		G_Conf.buffersize);
	G_Mixer.init();
	recorder::init();
}


/* -------------------------------------------------------------------------- */


void init_prepareKernelMIDI()
{
	kernelMidi::setApi(G_Conf.midiSystem);
	kernelMidi::openOutDevice(G_Conf.midiPortOut);
	kernelMidi::openInDevice(G_Conf.midiPortIn);
}


/* -------------------------------------------------------------------------- */


void init_prepareMidiMap()
{
	G_MidiMap.init_DEPR_();
	G_MidiMap.setDefault_DEPR_();
	G_MidiMap.readMap_DEPR_(G_Conf.midiMapPath);
}


/* -------------------------------------------------------------------------- */


void init_startGUI(int argc, char **argv)
{
	char win_label[32];
	sprintf(win_label, "%s - %s",
					G_APP_NAME,
					!strcmp(G_Patch_DEPR_.name, "") ? "(default patch)" : G_Patch_DEPR_.name);

	mainWin = new gdMainWindow(GUI_WIDTH, GUI_HEIGHT, win_label, argc, argv);
	mainWin->resize(G_Conf.mainWindowX, G_Conf.mainWindowY, G_Conf.mainWindowW, G_Conf.mainWindowH);

	/* never update the GUI elements if G_audio_status is bad, segfaults
	 * are around the corner */

	if (G_audio_status)
		gu_updateControls();

	if (!G_audio_status)
		gdAlert(
			"Your soundcard isn't configured correctly.\n"
			"Check the configuration and restart Giada."
		);
}

/* -------------------------------------------------------------------------- */


void init_startKernelAudio()
{
	if (G_audio_status)
		kernelAudio::startStream();

#ifdef WITH_VST
	G_PluginHost.allocBuffers();
#endif
}


/* -------------------------------------------------------------------------- */


void init_shutdown()
{
	G_quit = true;

	/* store position and size of the main window for the next startup */

	G_Conf.mainWindowX = mainWin->x();
	G_Conf.mainWindowY = mainWin->y();
	G_Conf.mainWindowW = mainWin->w();
	G_Conf.mainWindowH = mainWin->h();

	/* close any open subwindow, especially before cleaning PluginHost to
	 * avoid mess */

	gu_closeAllSubwindows();
	gLog("[init] all subwindows closed\n");

	/* write configuration file */

	if (!G_Conf.write())
		gLog("[init] error while saving configuration file!\n");
	else
		gLog("[init] configuration saved\n");

	/* if G_audio_status we close the kernelAudio FIRST, THEN the mixer.
	 * The opposite could cause random segfaults (even now with RtAudio?). */

	if (G_audio_status) {
		kernelAudio::closeDevice();
		G_Mixer.close();
		gLog("[init] Mixer closed\n");
	}

	recorder::clearAll();
	gLog("[init] Recorder cleaned up\n");

#ifdef WITH_VST
	G_PluginHost.freeAllStacks();
	gLog("[init] Plugin Host cleaned up\n");
#endif

	gLog("[init] Giada " G_VERSION_STR " closed\n\n");
	gLog_close();
}
