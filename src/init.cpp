/* ---------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * init
 *
 * ---------------------------------------------------------------------
 *
 * Copyright (C) 2010-2014 Giovanni A. Zuliani | Monocasual
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


#include "init.h"
#include "log.h"
#include "mixer.h"
#include "wave.h"
#include "const.h"
#include "utils.h"
#include "mixerHandler.h"
#include "patch.h"
#include "conf.h"
#include "pluginHost.h"
#include "recorder.h"
#include "gd_mainWindow.h"
#include "gui_utils.h"
#include "gd_warnings.h"
#include "kernelMidi.h"


extern Mixer 			   G_Mixer;
extern bool		 		   G_audio_status;
extern bool		 		   G_quit;
extern Patch     	   G_Patch;
extern Conf          G_Conf;
extern gdMainWindow *mainWin;

#ifdef WITH_VST
extern PluginHost	   G_PluginHost;
#endif


void init_prepareParser() {
	G_Conf.read();
	G_Patch.setDefault();
	gLog_init(LOG_MODE_STDOUT); /// TODO - use G_Conf values
	gLog("[init] Giada "VERSIONE"\n");
	gLog("[init] configuration file ready\n");
}


/* ------------------------------------------------------------------ */


void init_prepareKernelAudio() {
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


/* ------------------------------------------------------------------ */


void init_prepareKernelMIDI() {
	kernelMidi::setApi(G_Conf.midiSystem);
	kernelMidi::openOutDevice(G_Conf.midiPortOut);
	kernelMidi::openInDevice(G_Conf.midiPortIn);
}


/* ------------------------------------------------------------------ */


void init_startGUI(int argc, char **argv) {

	int x = (Fl::w() / 2) - (GUI_WIDTH / 2);
	int y = (Fl::h() / 2) - (GUI_HEIGHT / 2);

	char win_label[32];
	sprintf(win_label, "%s - %s",
					VERSIONE_STR,
					!strcmp(G_Patch.name, "") ? "(default patch)" : G_Patch.name);

	mainWin = new gdMainWindow(x, y, GUI_WIDTH, GUI_HEIGHT, win_label, argc, argv);

	/* never update the GUI elements if G_audio_status is bad, segfaults
	 * are around the corner */

	if (G_audio_status)
		gu_update_controls();

	if (!G_audio_status)
		gdAlert(
			"Your soundcard isn't configured correctly.\n"
			"Check the configuration and restart Giada."
		);
}

/* ------------------------------------------------------------------ */


void init_startKernelAudio() {
	if (G_audio_status)
		kernelAudio::startStream();

#ifdef WITH_VST
	G_PluginHost.allocBuffers();
#endif
}


/* ------------------------------------------------------------------ */


void init_shutdown() {

	G_quit = true;

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

	gLog("[init] Giada "VERSIONE" closed\n");
	gLog_close();
}
