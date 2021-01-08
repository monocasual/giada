/* -----------------------------------------------------------------------------
 *
 * Giada - Your Hardcore Loopmachine
 *
 * -----------------------------------------------------------------------------
 *
 * Copyright (C) 2010-2021 Giovanni A. Zuliani | Monocasual
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
#include "deps/json/single_include/nlohmann/json.hpp"
#include "gui/updater.h"
#include "utils/log.h"
#include "utils/fs.h"
#include "utils/time.h"
#include "utils/gui.h"
#include "utils/ver.h"
#include "gui/dialogs/mainWindow.h"
#include "gui/dialogs/warnings.h"
#include "glue/main.h"
#include "core/model/storage.h"
#include "core/model/model.h"
#include "core/channels/channelManager.h"
#include "core/mixer.h"
#include "core/wave.h"
#include "core/const.h"
#include "core/clock.h"
#include "core/mixerHandler.h"
#include "core/sequencer.h"
#include "core/patch.h"
#include "core/conf.h"
#include "core/waveManager.h"
#include "core/plugins/pluginManager.h"
#include "core/plugins/pluginHost.h"
#include "core/recorder.h"
#include "core/recorderHandler.h"
#include "core/recManager.h"
#include "core/midiMapConf.h"
#include "core/kernelMidi.h"
#include "core/kernelAudio.h"
#include "core/eventDispatcher.h"
#include "init.h"


extern giada::v::gdMainWindow* G_MainWin;


namespace giada::m::init
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

	model::load(conf::conf);
	
	if (!u::log::init(conf::conf.logMode))
		u::log::print("[init] log init failed! Using default stdout\n");

	if (midimap::read(conf::conf.midiMapPath) != MIDIMAP_READ_OK)
		u::log::print("[init] MIDI map read failed!\n");
}


/* -------------------------------------------------------------------------- */


void initSystem_()
{
	model::init();
	eventDispatcher::init();
}


/* -------------------------------------------------------------------------- */


void initAudio_()
{
	kernelAudio::openDevice();
	clock::init(conf::conf.samplerate, conf::conf.midiTCfps);
	mh::init();
	sequencer::init();
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

	v::updater::init();
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


/* -------------------------------------------------------------------------- */


void printBuildInfo_()
{
	u::log::print("[init] Giada %s\n", G_VERSION_STR);
	u::log::print("[init] Build date: " BUILD_DATE "\n");
#ifdef G_DEBUG_MODE
	u::log::print("[init] Debug build\n");
#else
	u::log::print("[init] Release build\n");
#endif
	u::log::print("[init] Dependencies:\n");
	u::log::print("[init]   FLTK - %d.%d.%d\n", FL_MAJOR_VERSION, FL_MINOR_VERSION, FL_PATCH_VERSION);
	u::log::print("[init]   RtAudio - %s\n", u::ver::getRtAudioVersion());
	u::log::print("[init]   RtMidi - %s\n", u::ver::getRtMidiVersion());
	u::log::print("[init]   Libsamplerate\n"); // TODO - print version
	u::log::print("[init]   Libsndfile - %s\n", u::ver::getLibsndfileVersion());
	u::log::print("[init]   JSON for modern C++ - %d.%d.%d\n",
		NLOHMANN_JSON_VERSION_MAJOR, NLOHMANN_JSON_VERSION_MINOR, NLOHMANN_JSON_VERSION_PATCH);
#ifdef WITH_VST
	u::log::print("[init]   JUCE - %d.%d.%d\n", JUCE_MAJOR_VERSION, JUCE_MINOR_VERSION, JUCE_BUILDNUMBER);
#endif
	kernelAudio::logCompiledAPIs();
}
} // {anonymous}


/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */
/* -------------------------------------------------------------------------- */


void startup(int argc, char** argv)
{
	printBuildInfo_();
	
	initConf_();
	initSystem_();
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

	model::init();
	channelManager::init();
	waveManager::init();
	clock::init(conf::conf.samplerate, conf::conf.midiTCfps);
	mh::init();
	sequencer::init();
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

	model::store(conf::conf);

	if (!conf::write())
		u::log::print("[init] error while saving configuration file!\n");
	else
		u::log::print("[init] configuration saved\n");

	shutdownAudio_();

	u::log::print("[init] Giada %s closed\n\n", G_VERSION_STR);
	u::log::close();
}
} // giada::m::init
