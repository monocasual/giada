/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2020 Giovanni A. Zuliani | Monocasual
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


#include <thread>
#include <atomic>
#include <ctime>
#ifdef __APPLE__
	#include <pwd.h>
#endif
#if (defined(__linux__) || defined(__FreeBSD__)) && defined(WITH_VST)
	#include <X11/Xlib.h> // For XInitThreads
#endif
#include <FL/Fl.H>
#include "gui/updater.h"
#include "utils/log.h"
#include "utils/fs.h"
#include "utils/time.h"
#include "utils/gui.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/warnings.h"
#include "glue/main.h"
#include "core/channels/channel.h"
#include "core/channels/channelManager.h"
#include "core/mixer.h"
#include "core/wave.h"
#include "core/const.h"
#include "core/clock.h"
#include "core/mixerHandler.h"
#include "core/patch.h"
#include "core/conf.h"
#include "core/waveManager.h"
#include "core/pluginManager.h"
#include "core/pluginHost.h"
#include "core/recorder.h"
#include "core/recorderHandler.h"
#include "core/recManager.h"
#include "core/midiMapConf.h"
#include "core/kernelMidi.h"
#include "core/kernelAudio.h"
#include "init.h"


extern giada::v::gdMainWindow* G_MainWin;


namespace giada {
namespace m {
namespace init
{
namespace
{
void initConf_()
{
	if (!conf::read())
		u::log::print("[init] Can't read configuration file! Using default values\n");
	
	patch::init();
	midimap::init();
	midimap::setDefault();
	
	if (!u::log::init(conf::conf.logMode))
		u::log::print("[init] log init failed! Using default stdout\n");

	if (midimap::read(conf::conf.midiMapPath) != MIDIMAP_READ_OK)
		u::log::print("[init] MIDI map read failed!\n");
}


/* -------------------------------------------------------------------------- */


void initAudio_()
{
	kernelAudio::openDevice();
	clock::init(conf::conf.samplerate, conf::conf.midiTCfps);
	mh::init();
	recorder::init();
	recorderHandler::init();

#ifdef WITH_VST

	pluginManager::init(conf::conf.samplerate, kernelAudio::getRealBufSize());
	pluginHost::init(kernelAudio::getRealBufSize());

#endif

	if (!kernelAudio::isReady())
		return;

	mixer::enable();
	kernelAudio::startStream();
}


/* -------------------------------------------------------------------------- */


void initMIDI_()
{
	kernelMidi::setApi(conf::conf.midiSystem);
	kernelMidi::openOutDevice(conf::conf.midiPortOut);
	kernelMidi::openInDevice(conf::conf.midiPortIn);	
}


/* -------------------------------------------------------------------------- */


void initGUI_(int argc, char** argv)
{
	/* This is of paramount importance on Linux with VST enabled, otherwise many
	plug-ins go nuts and crash hard. It seems that some plug-ins or our Juce-based
	PluginHost use Xlib concurrently. */
	
#if (defined(__linux__) || defined(__FreeBSD__)) && defined(WITH_VST)
	XInitThreads();
#endif

	G_MainWin = new v::gdMainWindow(G_MIN_GUI_WIDTH, G_MIN_GUI_HEIGHT, "", argc, argv);
	G_MainWin->resize(conf::conf.mainWindowX, conf::conf.mainWindowY, conf::conf.mainWindowW,
		conf::conf.mainWindowH);

	u::gui::updateMainWinLabel(patch::patch.name == "" ? G_DEFAULT_PATCH_NAME : patch::patch.name);
	
	if (!kernelAudio::isReady())
		v::gdAlert("Your soundcard isn't configured correctly.\n"
			"Check the configuration and restart Giada.");

	u::gui::updateStaticWidgets();

	Fl::add_timeout(G_GUI_REFRESH_RATE, v::updater::update, nullptr);
}


/* -------------------------------------------------------------------------- */


void shutdownAudio_()
{
	if (kernelAudio::isReady()) {
		kernelAudio::closeDevice();
		u::log::print("[init] KernelAudio closed\n");
		mh::close();
		u::log::print("[init] Mixer closed\n");
	}

	/* TODO - why cleaning plug-ins and mixer memory? Just shutdown the audio
	device and let the OS take care of the rest. */

#ifdef WITH_VST

	pluginHost::close();
	u::log::print("[init] PluginHost cleaned up\n");

#endif
}


/* -------------------------------------------------------------------------- */


void shutdownGUI_()
{
	u::gui::closeAllSubwindows();

	u::log::print("[init] All subwindows and UI thread closed\n");
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void startup(int argc, char** argv)
{
	time_t t;
	time (&t);
	u::log::print("[init] Giada %s - %s", G_VERSION_STR, ctime(&t));

	initConf_();
	initAudio_();
	initMIDI_();
	initGUI_(argc, argv);
}


/* -------------------------------------------------------------------------- */


void closeMainWindow()
{
	if (!v::gdConfirmWin("Warning", "Quit Giada: are you sure?"))
		return;

	G_MainWin->hide();
	delete G_MainWin;
}


/* -------------------------------------------------------------------------- */


void reset()
{	
	u::gui::closeAllSubwindows();
	G_MainWin->clearKeyboard(); 

	mh::close();
#ifdef WITH_VST
	pluginHost::close();
#endif

	channelManager::init();
	waveManager::init();
	clock::init(conf::conf.samplerate, conf::conf.midiTCfps);
	mh::init();
	recorder::init();
#ifdef WITH_VST
	pluginManager::init(conf::conf.samplerate, kernelAudio::getRealBufSize());
#endif

	
	u::gui::updateMainWinLabel(G_DEFAULT_PATCH_NAME);
	u::gui::updateStaticWidgets();
}


/* -------------------------------------------------------------------------- */


void shutdown()
{
	shutdownGUI_();

	if (!conf::write())
		u::log::print("[init] error while saving configuration file!\n");
	else
		u::log::print("[init] configuration saved\n");

	shutdownAudio_();

	u::log::print("[init] Giada %s closed\n\n", G_VERSION_STR);
	u::log::close();
}
}}} // giada::m::init
