/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2018 Giovanni A. Zuliani | Monocasual
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
#ifdef __APPLE__
	#include <pwd.h>
#endif
#if (defined(__linux__) || defined(__FreeBSD__)) && defined(WITH_VST)
	#include <X11/Xlib.h> // For XInitThreads
#endif
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
#include "clock.h"
#include "channel.h"
#include "mixerHandler.h"
#include "patch.h"
#include "conf.h"
#include "pluginHost.h"
#include "recorder.h"
#include "midiMapConf.h"
#include "kernelMidi.h"
#include "kernelAudio.h"


extern bool		 		   G_quit;
extern gdMainWindow *G_MainWin;


using namespace giada::m;


void init_prepareParser()
{
	time_t t;
  time (&t);
	gu_log("[init] Giada " G_VERSION_STR " - %s", ctime(&t));

	conf::read();
	patch::init();

	if (!gu_logInit(conf::logMode))
		gu_log("[init] log init failed! Using default stdout\n");

	gu_log("[init] configuration file ready\n");
}


/* -------------------------------------------------------------------------- */


void init_prepareKernelAudio()
{
  kernelAudio::openDevice();
  clock::init(conf::samplerate, conf::midiTCfps);
	mixer::init(clock::getFramesInLoop(), kernelAudio::getRealBufSize());
	recorder::init();

#ifdef WITH_VST

	/* If with Jack don't use buffer size stored in Conf. Use real buffersize
	from the soundcard (kernelAudio::realBufsize). */

	if (conf::soundSystem == G_SYS_API_JACK)
		pluginHost::init(kernelAudio::getRealBufSize(), conf::samplerate);
	else
		pluginHost::init(conf::buffersize, conf::samplerate);

	pluginHost::sortPlugins(conf::pluginSortMethod);

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
	midimap::init();
	midimap::setDefault();

	if (midimap::read(conf::midiMapPath) != MIDIMAP_READ_OK)
		gu_log("[init_prepareMidiMap] MIDI map read failed!\n");
}


/* -------------------------------------------------------------------------- */


void init_startGUI(int argc, char** argv)
{
	/* This is of paramount importance on Linux with VST enabled, otherwise many
	plug-ins go nuts and crash hard. It seems that some plug-ins or our Juce-based
	PluginHost use Xlib concurrently. */
	
#if (defined(__linux__) || defined(__FreeBSD__)) && defined(WITH_VST)
	XInitThreads();
#endif

	G_MainWin = new gdMainWindow(G_MIN_GUI_WIDTH, G_MIN_GUI_HEIGHT, "", argc, argv);
	G_MainWin->resize(conf::mainWindowX, conf::mainWindowY, conf::mainWindowW,
    conf::mainWindowH);

  gu_updateMainWinLabel(patch::name == "" ? G_DEFAULT_PATCH_NAME : patch::name);

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

	/* close any open subwindow, especially before cleaning PluginHost to
	 * avoid mess */

	gu_closeAllSubwindows();
	gu_log("[init] all subwindows closed\n");

	/* write configuration file */

	if (!conf::write())
		gu_log("[init] error while saving configuration file!\n");
	else
		gu_log("[init] configuration saved\n");

	recorder::clearAll();
	gu_log("[init] Recorder cleaned up\n");

#ifdef WITH_VST

	pluginHost::freeAllStacks(&mixer::channels, &mixer::mutex);
  pluginHost::close();
	gu_log("[init] PluginHost cleaned up\n");

#endif

	if (kernelAudio::getStatus()) {
		kernelAudio::closeDevice();
		gu_log("[init] KernelAudio closed\n");
		mixer::close();
		gu_log("[init] Mixer closed\n");
	}

	gu_log("[init] Giada " G_VERSION_STR " closed\n\n");
	gu_logClose();
}
