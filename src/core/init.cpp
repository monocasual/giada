/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * init
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


#include <ctime>
#include "../utils/log.h"
#include "../utils/fs.h"
#include "../utils/gui.h"
#include "../gui/dialogs/gd_mainWindow.h"
#include "../gui/dialogs/gd_warnings.h"
#include "../glue/main.h"
#include "init.h"
#include "mixer.h"
#include "wave.h"
#include "const.h"
#include "channel.h"
#include "mixerHandler.h"
#include "patch_DEPR_.h"
#include "patch.h"
#include "conf.h"
#include "pluginHost.h"
#include "recorder.h"
#include "midiMapConf.h"
#include "kernelMidi.h"
#include "kernelAudio.h"


extern Recorder  	   G_Recorder;
extern bool		 		   G_quit;
extern Patch_DEPR_   G_Patch_DEPR_;
extern Patch         G_Patch;
extern MidiMapConf   G_MidiMap;
extern gdMainWindow *G_MainWin;

#ifdef WITH_VST
extern PluginHost G_PluginHost;
#endif


using namespace giada;


void init_prepareParser()
{
	time_t t;
  time (&t);
	gu_log("[init] Giada " G_VERSION_STR " - %s", ctime(&t));

	conf::read();
	G_Patch_DEPR_.setDefault();
	G_Patch.init();

	if (!gu_logInit(conf::logMode))
		gu_log("[init] log init failed! Using default stdout\n");

	gu_log("[init] configuration file ready\n");
}


/* -------------------------------------------------------------------------- */


void init_prepareKernelAudio()
{
  kernelAudio::openDevice();
	mixer::init();
	G_Recorder.init();

#ifdef WITH_VST

	/* If with Jack don't use buffer size stored in Conf. Use real buffersize
	from the soundcard (kernelAudio::realBufsize). */

	if (conf::soundSystem == SYS_API_JACK)
		G_PluginHost.init(kernelAudio::getRealBufSize(), conf::samplerate);
	else
		G_PluginHost.init(conf::buffersize, conf::samplerate);

	G_PluginHost.sortPlugins(conf::pluginSortMethod);

#endif
}


/* -------------------------------------------------------------------------- */


void init_prepareKernelMIDI()
{
	kernelMidi::setApi(conf::midiSystem);
	kernelMidi::openOutDevice(conf::midiPortOut);
	kernelMidi::openInDevice(conf::midiPortIn);
}


/* -------------------------------------------------------------------------- */


void init_prepareMidiMap()
{
	G_MidiMap.init();
	G_MidiMap.setDefault_DEPR_();
	G_MidiMap.setDefault();

	/* read with deprecated method first. If it fails, try with the new one. */
	// TODO - do the opposite: if json fails, go with deprecated one

	if (G_MidiMap.read(conf::midiMapPath) != MIDIMAP_READ_OK) {
		gu_log("[init_prepareMidiMap] JSON-based midimap read failed, trying with the deprecated one...\n");
		if (G_MidiMap.readMap_DEPR_(conf::midiMapPath) == MIDIMAP_INVALID)
			gu_log("[init_prepareMidiMap] unable to read deprecated midimap. Nothing to do\n");
		}
}


/* -------------------------------------------------------------------------- */


void init_startGUI(int argc, char **argv)
{
	G_MainWin = new gdMainWindow(GUI_WIDTH, GUI_HEIGHT, "", argc, argv);
	G_MainWin->resize(conf::mainWindowX, conf::mainWindowY, conf::mainWindowW,
    conf::mainWindowH);

  gu_updateMainWinLabel(G_Patch.name == "" ? G_DEFAULT_PATCH_NAME : G_Patch.name);

	/* never update the GUI elements if kernelAudio::getStatus() is bad, segfaults
	 * are around the corner */

	if (kernelAudio::getStatus())
		gu_updateControls();
  else
		gdAlert("Your soundcard isn't configured correctly.\n"
			"Check the configuration and restart Giada.");
}

/* -------------------------------------------------------------------------- */


void init_startKernelAudio()
{
	if (kernelAudio::getStatus())
		kernelAudio::startStream();
}


/* -------------------------------------------------------------------------- */


void init_shutdown()
{
	G_quit = true;

	/* store position and size of the main window for the next startup */

	conf::mainWindowX = G_MainWin->x();
	conf::mainWindowY = G_MainWin->y();
	conf::mainWindowW = G_MainWin->w();
	conf::mainWindowH = G_MainWin->h();

	/* close any open subwindow, especially before cleaning PluginHost_DEPR_ to
	 * avoid mess */

	gu_closeAllSubwindows();
	gu_log("[init] all subwindows closed\n");

	/* write configuration file */

	if (!conf::write())
		gu_log("[init] error while saving configuration file!\n");
	else
		gu_log("[init] configuration saved\n");

	/* if kernelAudio::getStatus() we close the kernelAudio FIRST, THEN the mixer.
	 * The opposite could cause random segfaults (even now with RtAudio?). */

	if (kernelAudio::getStatus()) {
		kernelAudio::closeDevice();
		mixer::close();
		gu_log("[init] Mixer closed\n");
	}

	G_Recorder.clearAll();
  for (unsigned i=0; i<mixer::channels.size(); i++) {
		mixer::channels.at(i)->hasActions  = false;
		mixer::channels.at(i)->readActions = false;
		//if (mixer::channels.at(i)->type == CHANNEL_SAMPLE)
		//	((SampleChannel*)mixer::channels.at(i))->readActions = false;
	}
	gu_log("[init] Recorder cleaned up\n");

#ifdef WITH_VST
	G_PluginHost.freeAllStacks(&mixer::channels, &mixer::mutex_plugins);
	gu_log("[init] PluginHost cleaned up\n");
#endif

	gu_log("[init] Giada " G_VERSION_STR " closed\n\n");
	gu_logClose();
}
