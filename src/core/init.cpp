/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * init
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


#include <ctime>
#include "../utils/log.h"
#include "../utils/fs.h"
#include "../utils/gui.h"
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
extern Recorder  	   G_Recorder;
extern KernelMidi    G_KernelMidi;
extern bool		 		   G_audio_status;
extern bool		 		   G_quit;
extern Patch_DEPR_   G_Patch_DEPR_;
extern Patch         G_Patch;
extern Conf          G_Conf;
extern MidiMapConf   G_MidiMap;
extern gdMainWindow *G_MainWin;

#ifdef WITH_VST
extern PluginHost G_PluginHost;
#endif


void init_prepareParser()
{
	time_t t;
  time (&t);
	gu_log("[init] Giada " G_VERSION_STR " - %s", ctime(&t));

	G_Conf.read();
	G_Patch_DEPR_.setDefault();
	G_Patch.init();

#ifdef WITH_VST

	G_PluginHost.init(G_Conf.buffersize, G_Conf.samplerate);
	G_PluginHost.sortPlugins(G_Conf.pluginSortMethod);

#endif


	if (!gu_logInit(G_Conf.logMode))
		gu_log("[init] log init failed! Using default stdout\n");

	gu_log("[init] configuration file ready\n");
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
	G_Recorder.init();
}


/* -------------------------------------------------------------------------- */


void init_prepareKernelMIDI()
{
	G_KernelMidi.setApi(G_Conf.midiSystem);
	G_KernelMidi.openOutDevice(G_Conf.midiPortOut);
	G_KernelMidi.openInDevice(G_Conf.midiPortIn);
}


/* -------------------------------------------------------------------------- */


void init_prepareMidiMap()
{
	G_MidiMap.init();
	G_MidiMap.setDefault_DEPR_();
	G_MidiMap.setDefault();

	/* read with deprecated method first. If it fails, try with the new one. */
	// TODO - do the opposite: if json fails, go with deprecated one

	if (G_MidiMap.read(G_Conf.midiMapPath) != MIDIMAP_READ_OK) {
		gu_log("[init_prepareMidiMap] JSON-based midimap read failed, trying with the deprecated one...\n");
		if (G_MidiMap.readMap_DEPR_(G_Conf.midiMapPath) == MIDIMAP_INVALID)
			gu_log("[init_prepareMidiMap] unable to read deprecated midimap. Nothing to do\n");
		}
}


/* -------------------------------------------------------------------------- */


void init_startGUI(int argc, char **argv)
{
	char win_label[32];
	sprintf(win_label, "%s - %s",
					G_APP_NAME,
					!strcmp(G_Patch_DEPR_.name, "") ? "(default patch)" : G_Patch_DEPR_.name);

	G_MainWin = new gdMainWindow(GUI_WIDTH, GUI_HEIGHT, win_label, argc, argv);
	G_MainWin->resize(G_Conf.mainWindowX, G_Conf.mainWindowY, G_Conf.mainWindowW, G_Conf.mainWindowH);

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
}


/* -------------------------------------------------------------------------- */


void init_shutdown()
{
	G_quit = true;

	/* store position and size of the main window for the next startup */

	G_Conf.mainWindowX = G_MainWin->x();
	G_Conf.mainWindowY = G_MainWin->y();
	G_Conf.mainWindowW = G_MainWin->w();
	G_Conf.mainWindowH = G_MainWin->h();

	/* close any open subwindow, especially before cleaning PluginHost_DEPR_ to
	 * avoid mess */

	gu_closeAllSubwindows();
	gu_log("[init] all subwindows closed\n");

	/* write configuration file */

	if (!G_Conf.write())
		gu_log("[init] error while saving configuration file!\n");
	else
		gu_log("[init] configuration saved\n");

	/* if G_audio_status we close the kernelAudio FIRST, THEN the mixer.
	 * The opposite could cause random segfaults (even now with RtAudio?). */

	if (G_audio_status) {
		kernelAudio::closeDevice();
		G_Mixer.close();
		gu_log("[init] Mixer closed\n");
	}

	G_Recorder.clearAll();
	gu_log("[init] Recorder cleaned up\n");

#ifdef WITH_VST
	G_PluginHost.freeAllStacks(&G_Mixer.channels, &G_Mixer.mutex_plugins);
	gu_log("[init] PluginHost cleaned up\n");
#endif

	gu_log("[init] Giada " G_VERSION_STR " closed\n\n");
	gu_logClose();
}
